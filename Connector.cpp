//=====================================================================
//
// Connector.cpp - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 21:24:47
//
//=====================================================================
#include "Connector.h"
#include "Logger.h"
#include "SocketsOps.h"
#include "Channel.h"

#include <assert.h>
using namespace simple_muduo;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
	: loop_(loop),
	serverAddr_(serverAddr),
	connect_(false),
	state_(kDisconnected),
	retryDelayMs_(kInitRetryDelayMs)
{
	LOG_INFO("Connector \n");
}

Connector::~Connector()
{
}

void Connector::start()
{
	connect_ = true;
	// FIXME: unsafe
	loop_->runInLoop(std::bind(&Connector::startInLoop, this)); 
}

void Connector::startInLoop()
{
	loop_->isInLoopThread();
	assert(state_ == kDisconnected);
	if (connect_)
	{
		connect();
	}
	else
	{
		LOG_ERROR("do not connect\n");
	}
}

void Connector::stop()
{
	connect_ = false;
	// FIXME: unsafe
	loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); 
}

void Connector::stopInLoop()
{
	loop_->isInLoopThread();
	if (state_ == kConnecting)
	{
		setState(kDisconnected);
		int sockfd = removeAndResetChannel();
		retry(sockfd);
	}
}

void Connector::connect()
{
	int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
	int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
	int savedErrno = (ret == 0) ? 0 : errno;
	switch (savedErrno)
	{
		case 0:
		case EINPROGRESS:
		case EINTR:
		case EISCONN:
			connecting(sockfd);
			break;

		case EAGAIN:
		case EADDRINUSE:
		case EADDRNOTAVAIL:
		case ECONNREFUSED:
		case ENETUNREACH:
			retry(sockfd);
			break;

		case EACCES:
		case EPERM:
		case EAFNOSUPPORT:
		case EALREADY:
		case EBADF:
		case EFAULT:
		case ENOTSOCK:
			LOG_INFO("connect error in Connector::startInLoop \n");
			sockets::close(sockfd);
			break;

		default:
			LOG_INFO("Unexpected error in Connector::startInLoop\n");
			sockets::close(sockfd);
			// connectErrorCallback_();
			break;
	}
}

void Connector::restart()
{
	loop_->isInLoopThread();
	setState(kDisconnected);
	retryDelayMs_ = kInitRetryDelayMs;
	connect_ = true;
	startInLoop();
}

void Connector::connecting(int sockfd)
{
	setState(kConnecting);
	assert(!channel_);
	channel_.reset(new Channel(loop_, sockfd));
	channel_->setWriteCallback(
			std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
	channel_->setErrorCallback(
			std::bind(&Connector::handleError, this)); // FIXME: unsafe

	// channel_->tie(shared_from_this()); is not working,
	// as channel_ is not managed by shared_ptr
	channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
	channel_->disableAll();
	channel_->remove();
	int sockfd = channel_->fd();
	// Can't reset channel_ here, because we are inside Channel::handleEvent
	loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
	return sockfd;
}

void Connector::resetChannel()
{
	channel_.reset();
}

void Connector::handleWrite()
{
	LOG_INFO("Connector::handleWrite\n");
	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		if (err)
		{
			retry(sockfd);
		}
		else if (sockets::isSelfConnect(sockfd))
		{
			retry(sockfd);
		}
		else
		{
			setState(kConnected);
			if (connect_)
			{
				newConnectionCallback_(sockfd);
			}
			else
			{
				sockets::close(sockfd);
			}
		}
	}
	else
	{
		// what happened?
		assert(state_ == kDisconnected);
	}
}

void Connector::handleError()
{
	// LOG_ERROR << "Connector::handleError state=" << state_;
	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		LOG_INFO("errno:%d\n", err);
		retry(sockfd);
	}
}

void Connector::retry(int sockfd)
{
	sockets::close(sockfd);
	setState(kDisconnected);
	if (connect_)
	{
		LOG_INFO("Connector::retry:Retry connecting to [%s]", serverAddr_.toIpPort().c_str());
		loop_->runInLoop(std::bind(&Connector::startInLoop, this));
	}
	else
	{
		LOG_INFO("do not connect\n");
	}
}
