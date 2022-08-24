//=====================================================================
//
// TcpClient.h - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 19:15:55
//
//=====================================================================
#ifndef SIMPLE_MUDUO_TCPCLIENT_H
#define SIMPLE_MUDUO_TCPCLIENT_H
#include <memory>
#include <mutex>

#include "Callbacks.h"
#include "Connector.h"
#include "EventLoop.h"
#include "noncopyable.h"
#include "TcpConnection.h"

namespace simple_muduo{

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;
// TcpClient类是用Connetor去建立连接，用TcpConnection去维护管理连接，一对一
// TcpConnection断开连接后，Connector具备反复重连机制，因此服务端和客户端的启动顺序无先后
// Tcp自连接问题：mudduo采用sockets::isSelfConnect(in sockfd)方法解决。
// tcp的断开，重连处理
class TcpClient : noncopyable
{
public:
  TcpClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const std::string& nameArg);
  ~TcpClient();
  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection();

  EventLoop* getLoop() const {return loop_;}
  bool retry() const {return retry_;}
  void enableRetry() {retry_ = true;}

  const std::string& name() const {return name_;}
  void setConnectionCallback(ConnectionCallback cb){connectionCallback_ = std::move(cb);}
  void setMessageCallback(MessageCallback cb){messageCallback_ = std::move(cb);}
  void setWriteCompleteCallback(WriteCompleteCallback cb) {writeCompleteCallback_ = std::move(cb);}
private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr& conn);
private:
  EventLoop* loop_;
  const std::string name_;

  ConnectorPtr connector_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;
  bool connect_;
  // always in loop thread
  int nextConnId_;
  std::mutex mutex_;
  TcpConnectionPtr connection_;
};
}
#endif // SIMPLE_MUDUO_TCPCLIENT_H
