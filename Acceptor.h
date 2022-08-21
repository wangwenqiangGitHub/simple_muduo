//=====================================================================
//
// Acceptor.h - 
//
// Created by wwq on 2022/08/20
// Last Modified: 2022/08/20 11:11:30
//
//=====================================================================
#ifndef SIMPLE_MUDUO_ACCEPTOR_H
#define SIMPLE_MUDUO_ACCEPTOR_H
#include <functional>
#include "Channel.h"
#include "Socket.h"
namespace simple_muduo {

class EventLoop;
class InetAddress;
// 客户端连接的接收器
// Acceptor中的loop就是用户建立的eventLoop也就是主loop即baseLoop
// 当监听的acceptSocket_上有连接发生就会调用handleRead()回调函数
class Acceptor
{
public:
	using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& )>;
	Acceptor(EventLoop *loop,const InetAddress& listenAdd, bool reuseport);
	~Acceptor();

	// TcpServer中用到了这个回调
	void setNewConnctionCallback(const NewConnectionCallback& cb){}
    bool listenning() const { return listenning_; }
    void listen();
private:
	void handleRead();
private:
	// Acceptor用的就是用户定义的那个baseLoop 也称作mainLoop
	EventLoop *loop_;
	Socket acceptSocket_;
	Channel acceptChannel_;
	NewConnectionCallback  NewConnectionCallback_;
	bool listenning_;
	// 提前占一个文件描述。当文件描述符达到上线后，关闭这个fd，再接收
	int idleFd_;
};

} // namespace simple_muduo
#endif // SIMPLE_MUDUO_ACCEPTOR_H
