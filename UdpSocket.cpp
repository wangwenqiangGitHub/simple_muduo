#include "UdpSocket.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "Channel.h"
#include "Socket.h"
#include "Timestamp.h"
#include <netinet/in.h>
#include <sys/socket.h>


using namespace simple_muduo;

UdpSocket::UdpSocket(EventLoop* loop, std::string ip, uint16_t port)
	: loop_(loop)
	, ip_(ip)
	, port_(port)
	// , is_connect_peer_(false)
{}

UdpSocket::~UdpSocket()
{
	int fd = channel_->fd();
	if(fd > 0)
	{
		sockets::close(fd);
	}
}

void UdpSocket::start()
{
	fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
	int opt = 1;
	setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, static_cast<socklen_t>(opt));
	setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, (char*)&opt, static_cast<socklen_t>(opt));
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_.c_str());
	addr.sin_port = ::htons(port_);
	int ret = ::bind(fd_, (struct sockaddr*)&addr, sizeof addr);
	if(ret != 0)
	{
		printf("bind dup erro\n");
	}
	{
		struct sockaddr addr;
		struct sockaddr_in* addr_v4;
		socklen_t addr_len = sizeof(addr);
	if(0 == ::getsockname(fd_, &addr, &addr_len))
	{
		if(addr.sa_family == AF_INET)
		{
			addr_v4 = (sockaddr_in*)& addr;
			port_ = ntohs(addr_v4->sin_port);
		}
	}
	}

	channel_.reset(new Channel(loop_, fd_));
	channel_->setReadCallback([this](Timestamp time){this->handleRead();});
	channel_->enableReading();
}

int UdpSocket::send(const uint8_t *buf, size_t len, const struct sockaddr_in &peer_addr)
{
	int ret = 
		sendto(fd_, (const char*)buf, len, 0,(struct sockaddr *)&peer_addr_, sizeof(peer_addr_));
	if(ret < 0)
	{
		printf("send eror\n");
		return -1;
	}
	return ret;
}

void UdpSocket::handleRead()
{
	struct sockaddr_in peerAddr;
	unsigned int nAddrLen=sizeof(peerAddr);
	int recvLen = ::recvfrom(fd_, buf_,kBufferSize, 0, (struct sockaddr*)&peerAddr, &nAddrLen);
	if(read_callbak_)
	{
		read_callbak_(buf_,recvLen,&peerAddr);
	}
}
