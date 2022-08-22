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
#include <mutex>

#include "Callbacks.h"
#include "EventLoop.h"
#include "noncopyable.h"
#include "TcpConnection.h"

namespace simple_muduo{
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

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;
  bool connect_;
  std::mutex mutex_;
  TcpConnectionPtr connection_;
};
}
#endif // SIMPLE_MUDUO_TCPCLIENT_H
