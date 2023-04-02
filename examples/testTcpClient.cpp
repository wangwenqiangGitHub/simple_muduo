#include "../Logger.h"
#include "../EventLoopThread.h"
#include "../TcpClient.h"
#include "../SocketsOps.h"
#include "../Buffer.h"
#include "../EventLoop.h"

#include <iostream>
#include <mutex>
#include <stdio.h>
#include <string>
#include <unistd.h>

using namespace simple_muduo;

class ChatClient : noncopyable
{
 public:
  ChatClient(EventLoop* loop, const InetAddress& serverAddr)
    : client_(loop, serverAddr, "ChatClient")
      // codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
  {
    client_.setConnectionCallback(
        std::bind(&ChatClient::onConnection, this, std::placeholders::_1));
    client_.setMessageCallback(
        // std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
        std::bind(&ChatClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  void disconnect()
  {
    client_.disconnect();
  }

  void write(const std::string& message)
  {
	  std::unique_lock<std::mutex> lock(mutex_);
    if (connection_)
    {
      // codec_.send(get_pointer(connection_), message);
    Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = sockets::hostToNetwork32(len);
    buf.prepend(&be32, sizeof be32);
	std::string msg(buf.peek(), buf.readableBytes());
    connection_->send(msg);
    }
  }
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime)
  {
    while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
    {
      // FIXME: use Buffer::peekInt32()
      const void* data = buf->peek();
      int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
      const int32_t len = sockets::networkToHost32(be32);
      if (len > 65536 || len < 0)
      {
        // LOG_ERROR << "Invalid length " << len;
        conn->shutdown();  // FIXME: disable reading
        break;
      }
      else if (buf->readableBytes() >= len + kHeaderLen)
      {
        buf->retrieve(kHeaderLen);
        std::string message(buf->peek(), len);
        onStringMessage(conn, message, receiveTime);
        buf->retrieve(len);
      }
      else
      {
        break;
      }
    }
  }
 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
	LOG_INFO("local port: %s; peer port:%s", 
			conn->localAddress().toIpPort().c_str(),
			conn->peerAddress().toIpPort().c_str());
	
	std::unique_lock<std::mutex> lock(mutex_);
    if (conn->connected())
    {
      connection_ = conn;
    }
    else
    {
      connection_.reset();
    }
  }

  void onStringMessage(const TcpConnectionPtr&,
                       const std::string& message,
                       Timestamp)
  {
    printf("<<< %s\n", message.c_str());
  }

  int kHeaderLen = 4;
  TcpClient client_;
  // LengthHeaderCodec codec_;
  std::mutex mutex_;
  TcpConnectionPtr connection_;
};

int main(int argc, char* argv[])
{
  // LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      client.write(line);
    }
    client.disconnect();
    // CurrentThread::sleepUsec(1000*1000);  // wait for disconnect, see ace/logging/client.cc
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}

