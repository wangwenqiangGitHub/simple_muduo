#include <cstddef>
#include <functional>
#include <sys/types.h>

#include "Callbacks.h"
#include "Channel.h"
#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "EventLoop.h"
#include "Timestamp.h"

using namespace simple_muduo;

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_INFO("%s:%s:%d mainLoop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,
							const std::string& nameArg,
							int sockfd,
							const InetAddress& localAddr,
							const InetAddress& peerAddr)
	: loop_(CheckLoopNotNull(loop))
	, name_(nameArg)
	, state_(kConnecting)
	, reading_(true)
	, socket_(new Socket(sockfd))
	, channel_(new Channel(loop, sockfd))
	, localAddr_(localAddr)
	, peerAddr_(peerAddr)
	, highWaterMark_(64 * 1024 * 1024) //64M
{
	// poller监听的事件发生了就通知channel触发相应的回调
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
	
	LOG_INFO("TcpConnection::name[%s], fd:%d\n", name_.c_str(), sockfd);
	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::send(const std::string &buf)
{
	if(state_ == kConnected)
	{
		if(state_ == kConnected)
		{
			// 这种是对于单个reactor的情况 
			// 用户调用conn->send时 loop_即为当前线程
			if(loop_->isInLoopThread())
			{
				sendInLoop(buf.c_str(), buf.size());
			}
			else
			{
				loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this, buf.c_str(), buf.size()));
			}
		}
	}
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
	ssize_t nwrote = 0;
	ssize_t remaining = len;
	bool faultError = false;
	// 之前调用过该connection的shutdown 不能再进行发送了
	if(state_ == kDisconnected)
	{
		LOG_INFO("disconnected give up writing\n");
	}

	// 表示channel_第一次开始写数据或者缓冲区没有待发送数据
	if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
	{
		nwrote = ::write(channel_->fd(), data, len);
		if(nwrote >= 0)
		{
			remaining = len - nwrote;
			if(remaining == 0 && writeCompleteCallback_)
			{
                // 既然在这里数据全部发送完成，就不用再给channel设置epollout事件了
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this()));
			}
		}
		else
		{
            nwrote = 0;
            if (errno != EWOULDBLOCK) // EWOULDBLOCK表示非阻塞情况下没有数据后的正常返回 等同于EAGAIN
            {
                LOG_ERROR("TcpConnection::sendInLoop");
				// SIGPIPE RESET
                if (errno == EPIPE || errno == ECONNRESET) 
                {
                    faultError = true;
                }
            }
		}
	}

    // 说明当前这一次write并没有把数据全部发送出去 剩余的数据需要保存到缓冲区当中
    // 然后给channel注册EPOLLOUT事件，Poller发现tcp的发送缓冲区有空间后会通知
    // 相应的sock->channel，调用channel对应注册的writeCallback_回调方法，
    // channel的writeCallback_实际上就是TcpConnection设置的handleWrite回调，
    // 把发送缓冲区outputBuffer_的内容全部发送完成
    if (!faultError && remaining > 0)
    {
        // 目前发送缓冲区剩余的待发送的数据的长度
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append((char *)data + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting(); // 这里一定要注册channel的写事件 否则poller不会给channel通知epollout
        }
    }
}

void TcpConnection::shutdown()
{
	if(state_ == kConnected)
	{
		setState(kDisconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
	if(!channel_->isWriting())
	{
		socket_->shutdownWrite();
	}
}

// 连接建立
void TcpConnection::connectEstablished()
{
	setState(kConnected);
	channel_->tie(shared_from_this());
	// 向poller注册channel的EPOLLIN读事件
	channel_->enableWriting();

	// 新连接建立 执行回调
	connectionCallback_(shared_from_this());
}

// 连接销毁
void TcpConnection::connectDestroyed()
{
	if(state_ == kConnected)
	{
		setState(kDisconnected);
		// channel监听的fd 把channel的所有感兴趣的事件从poller中删除掉
		channel_->disableAll();
		connectionCallback_(shared_from_this());
	}
	// 把channel_从poller中删除掉
	channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
	if(n>0)
	{
        // 已建立连接的用户有可读事件发生了 调用用户传入的回调操作onMessage shared_from_this就是获取了TcpConnection的智能指针
		messageCallback_(shared_from_this(),&inputBuffer_, receiveTime);
	}
	else if(n == 0)
	{
		handleClose();
	}
	else
	{
		errno = savedErrno;
		LOG_INFO("TcpConnection::handleRead errno\n");
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	if(channel_->isWriting())
	{
		int savedErrno = 0;
		ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
		if(n > 0)
		{
			outputBuffer_.retrieve(n);
			if(outputBuffer_.readableBytes() == 0)
			{
				channel_->disableWriting();
				if(writeCompleteCallback_)
				{
                    // TcpConnection对象在其所在的subloop中 向pendingFunctors_中加入回调
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
				}
				if(state_ == kDisconnecting)
				{
					// 在当前所属的loop中把TcpConnection删除掉
					shutdownInLoop();
				}
			}
		}
		else
		{
			LOG_INFO("error\n");
		}
	}
	else 
	{
		LOG_INFO("Tcpconnection fd=%d is down, no more writing\n",channel_->fd());
	}
}

void TcpConnection::handleClose()
{
	LOG_INFO("fd=%d; state=%d\n",channel_->fd(),(int)state_);
	setState(kDisconnected);
	channel_->disableAll();

	TcpConnectionPtr connPtr(shared_from_this());
	// 执行连接关闭的回调
	connectionCallback_(connPtr);
	// 执行关闭连接的回调
	// 执行的是TcpServer::removeConnection回调方法   
	// must be the last line
	closeCallback_(connPtr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}
