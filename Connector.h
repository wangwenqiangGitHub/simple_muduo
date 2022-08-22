//=====================================================================
//
// Connector.h - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 20:55:38
//
//=====================================================================
#ifndef SIMPLE_MUDUO_CONNECTOR_H
#define SIMPLE_MUDUO_CONNECTOR_H
#include "EventLoop.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include <functional>
namespace simple_muduo {
class Connector : noncopyable
{
public:
	using NewConnectionCallback = std::function<void (int sockfd)>;
	
	Connector(EventLoop* loop, const InetAddress& serverAddr);
	~Connector();

	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{newConnectionCallback_ = cb;}

	void start();  // can be called in any thread
	void restart();  // must be called in loop thread
	void stop();  // can be called in any thread
private:
  enum States { kDisconnected, kConnecting, kConnected };

  void setState(States s) { state_ = s; }
  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();
private:
  static const int kMaxRetryDelayMs = 30*1000;
  static const int kInitRetryDelayMs = 500;

  EventLoop* loop_;
  InetAddress serverAddr_;
  bool connect_; // atomic
  States state_;  // FIXME: use atomic variable
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback newConnectionCallback_;
  int retryDelayMs_;
};
}
#endif // SIMPLE_MUDUO_CONNECTOR_H
