//=====================================================================
//
// TcpConnection.h - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 10:39:55
//
//=====================================================================
#ifndef SIMPLE_MUDUO_TCPCONNECTION_H
#define SIMPLE_MUDUO_TCPCONNECTION_H

#include "InetAddress.h"
#include "Timestamp.h"
#include "noncopyable.h"
#include "Callbacks.h"
#include "Buffer.h"

#include <cstddef>
#include <memory>
#include <atomic> // atomic_int

namespace simple_muduo{

class Channel;
class EventLoop;
class Socket;
// TcpConnection类 默认用shared_ptr来管理 服务器与客户端之间连接时使用
// 内部类，不提供用户使用，所提供的接口给TcpServer和TcpClient使用
// TcpConnection采用智能指针传递时，由于enable_shared_from_this会使引用计数加1
// TcpServer调用Acceptor监测到有新用户连接，通过accept函数可以得到connfd
// 通过=> TcpConnection设置回调 => 设置到Channel => Poller => Channel回调
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop *loop,
				const std::string& nameArg,
				int sockfd,
				const InetAddress& localAddr,
				const InetAddress& peerAddr);
	~TcpConnection();
	
	EventLoop* getLoop()const {return loop_;}
	const std::string &name() const {return name_;}
	const InetAddress& localAddress() const {return localAddr_;}
	const InetAddress& peerAddress() const {return peerAddr_;}

	bool connected() const {return state_ == kConnected;}
	void send(const std::string& buf);
	// 关闭连接
	void shutdown();

	// 设置回调函数
	void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb;}
	void setMessageCallback(const MessageCallback& cb){messageCallback_ = cb;}
	void setWriteCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_ = cb;}
	void setCloseCallback(const CloseCallback& cb){closeCallback_ = cb;}
	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark){ highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark;}

	// 建立连接
	void connectEstablished();
	// 连接销毁
	void connectDestroyed();
private:
	enum StateE
	{
		kDisconnected, // 断开连接
		kConnecting,   // 正在连接
		kConnected,    // 已连接
		kDisconnecting // 正在断开连接
	};

	void setState(StateE state) {state_ = state;}
	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();
	void sendInLoop(const void* data, size_t len);
	void shutdownInLoop();
private:
	// 这里是baseloop还是subloop由TcpServer中创建的线程数决定 若为多Reactor 该loop_指向subloop 若为单Reactor 该loop_指向baseloop
    EventLoop *loop_; 
	const std::string name_;
	std::atomic_int state_;
	bool reading_;

	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;

	const InetAddress localAddr_;
	const InetAddress peerAddr_;

    // 这些回调TcpServer也有 用户通过写入TcpServer注册 TcpServer再将注册的回调传递给TcpConnection TcpConnection再将回调注册到Channel中
    ConnectionCallback connectionCallback_;       // 有新连接时的回调
    MessageCallback messageCallback_;             // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;

    // 数据缓冲区
    Buffer inputBuffer_;    // 接收数据的缓冲区
    Buffer outputBuffer_;   // 发送数据的缓冲区 用户send向outputBuffer_发
};
}

#endif // SIMPLE_MUDUO_TCPCONNECTION_H
