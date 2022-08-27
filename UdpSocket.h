//=====================================================================
//
// UdpSocket.h - 
//
// Created by wwq on 2022/08/25
// Last Modified: 2022/08/25 22:11:59
//
//=====================================================================
#ifndef SIMPLE_MUDUO_UDPSOCKET_H
#define SIMPLE_MUDUO_UDPSOCKET_H
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <netinet/in.h>

namespace simple_muduo
{
#define kBufferSize 4096
class Channel;
class EventLoop;
using UdpSocketReadCallback = std::function<void(char* buf, int len, struct sockaddr_in* peerAddr)>;
class UdpSocket : public std::enable_shared_from_this<UdpSocket>
{
public:
	using ptr = std::shared_ptr<UdpSocket>;
	UdpSocket(EventLoop* loop, std::string ip, uint16_t port = 0);
	~UdpSocket();
	void bindPeer(const struct sockaddr_in& peerAddr){
		// is_connect_peer_ = true;
		peer_addr_ = peerAddr;
	}
	void start();
	void setReadCallback(UdpSocketReadCallback cb) {read_callbak_ =cb;}
	int send(const uint8_t* buf, size_t len, const struct sockaddr_in& peer_addr);
	uint16_t getPort(){return port_;}
	std::string getIP(){return ip_;}
private:
	void handleRead();
private:
	sockaddr_in peer_addr_;
	// bool is_connect_peer_;
	UdpSocketReadCallback read_callbak_;
	int fd_;
	uint16_t port_;
	std::string ip_;
	std::shared_ptr<Channel> channel_;
	EventLoop* loop_;
	char buf_[kBufferSize];
};
}
#endif // SIMPLE_MUDUO_UDPSOCKET_H
