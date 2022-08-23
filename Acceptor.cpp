#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logger.h"

using namespace simple_muduo;

static int createNonblocking()
{
	int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if(sockfd < 0)
	{
		LOG_ERROR("listen socket create err:%d\n",errno);
	}
	return sockfd;
}


Acceptor::Acceptor(EventLoop* loop, const InetAddress &listenAddr, bool reuseport)
	: loop_(loop)
	, acceptSocket_(createNonblocking())
	, acceptChannel_(loop, acceptSocket_.fd())
	, listenning_(false)
	, idleFd_(::open("dev/null", O_RDONLY|O_CLOEXEC))
{
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.setReusePort(true);
	acceptSocket_.bindAddress(listenAddr);
    // TcpServer::start() => Acceptor.listen() 如果有新用户连接
	// 要执行一个回调(accept => connfd => 打包成Channel => 唤醒subloop)
    // baseloop监听到有事件发生 => acceptChannel_(listenfd) => 执行该回调函数
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
	// 把从Poller中感兴趣的事件删除掉
	acceptChannel_.disableAll();
	// 调用EventLoop->removeChannel => Poller->removeChannel 把Poller的ChannelMap对应的部分删除
    acceptChannel_.remove();
}

void Acceptor::listen()
{
	listenning_ = true;
	// 监听
	acceptSocket_.listen();
	// acceptChannel_注册至Poller
	acceptChannel_.enableReading();
}

// listenfd有事件发生了，就是有新用户连接了
// handleRead()回调执行了，就是listenfd上有事件发生了
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (NewConnectionCallback_)
        {
            NewConnectionCallback_(connfd, peerAddr); // 轮询找到subLoop 唤醒并分发当前的新客户端的Channel
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
		// to many open file 文件描述符过多，一般设置是1024个，ulimit -a 可以查看
        if (errno == EMFILE)
        {
			::close(idleFd_);
			idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
