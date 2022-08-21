//=====================================================================
//
// Socket.h - 
//
// Created by wwq on 2022/08/17
// Last Modified: 2022/08/17 21:19:44
//
//=====================================================================
#ifndef SIMPLE_MUDUO_SOCKET_H_
#define SIMPLE_MUDUO_SOCKET_H_
#include "noncopyable.h"
namespace simple_muduo{

class InetAddress;

// 线程安全 封装soket fd
class Socket : noncopyable
{
public:
	explicit Socket(int sockfd)
		: sockfd_(sockfd)
	{}
	~Socket();
	int fd() const {return sockfd_;}
	void bindAddress(const InetAddress& localaddr);
	void listen();
	
	int accept(InetAddress* peeraddr);

	void shutdownWrite();

	void setTcpNoDelay(bool on);
	void setReuseAddr(bool on);
	void setReusePort(bool on);
	void setKeepAlive(bool on);
private:
	int sockfd_;
};
}
#endif // SIMPLE_MUDUO_SOCKET_H_
