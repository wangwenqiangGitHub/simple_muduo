//=====================================================================
//
// Socket.cpp - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 08:04:46
//
//=====================================================================
#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <unistd.h> // close()
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cstring> // memset


#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"

using namespace simple_muduo;
Socket::~Socket()
{
	::close(sockfd_);
}

void Socket:: bindAddress(const InetAddress& localaddr)
{
	if(0 != bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in)))
	{
		LOG_ERROR("bind erro\n");
	}
}

void Socket:: listen()
{
	if(0 != ::listen(sockfd_, 1024))
	{
		LOG_ERROR("listen erro\n");
	}
}

int  Socket:: accept(InetAddress* peeraddr)
{
    /**
     * 1. accept函数的参数不合法
     * 2. 对返回的connfd没有设置非阻塞
     * Reactor模型 one loop per thread
     * poller + non-blocking IO
     **/
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ::memset(&addr, 0, sizeof(addr));
    // fixed : int connfd = ::accept(sockfd_, (sockaddr *)&addr, &len);
    int connfd = ::accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket:: shutdownWrite()
{
	if(::shutdown(sockfd_, SHUT_WR) < 0)
	{
		LOG_ERROR("shutdownWrite error\n");
	}
}

void Socket:: setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

void Socket:: setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

void Socket:: setReusePort(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket:: setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}
