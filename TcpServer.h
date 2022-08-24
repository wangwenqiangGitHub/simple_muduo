//=====================================================================
//
// TcpServer.h - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 14:25:16
//
//=====================================================================
#ifndef SIMPLE_MUDUO_TCPSERVER_H
#define SIMPLE_MUDUO_TCPSERVER_H

// 提供用户使用的TcpServer
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

namespace simple_muduo{
class TcpServer
{
public:
	using ThreadInitCallback = std::function<void(EventLoop*)>;
	enum Option
	{
		kNoReusePort,
		kReusePort,
	};
	TcpServer(EventLoop* loop,
			  const InetAddress& listenAddr,
			  const std::string& nameArg,
			  Option option = kNoReusePort);
	~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    // 设置底层subloop的个数
    void setThreadNum(int numThreads);

	std::shared_ptr<EventLoopThreadPool> threadPool()
	{return threadPool_;}

    // 开启服务器监听
    void start();

	const std::string& ipPort() const {return ipPort_;}
	const std::string& name() const {return name_;}
	EventLoop* getLoop() const { return loop_; }
private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
private:
    EventLoop *loop_;

	const std::string ipPort_;
	const std::string name_;
	
	// 运行在mainloop 任务就是监听新连接事件
	std::unique_ptr<Acceptor> acceptor_;
	// one loop per thread
	std::shared_ptr<EventLoopThreadPool> threadPool_;

	// 新连接的回调
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;

	ThreadInitCallback threadInitCallback_;
	std::atomic_int started_;

	int nextConnId_;
	// 保存所有的连接
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
	ConnectionMap connections_;
};

} // namespace simple_muduo
#endif //SIMPLE_MUDUO_TCPSERVER_H
