#ifndef MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
#define MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H

#include "../../Logger.h"
#include "../../Buffer.h"
#include "../../SocketsOps.h"
#include "../../TcpConnection.h"


using namespace simple_muduo;
class LengthHeaderCodec : noncopyable
{
 public:
  typedef std::function<void (const TcpConnectionPtr&,
                                const std::string& message,
                                Timestamp)> StringMessageCallback;

  explicit LengthHeaderCodec(const StringMessageCallback& cb)
    : messageCallback_(cb)
  {
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
		LOG_INFO("Invalid length\n");
        conn->shutdown();  // FIXME: disable reading
        break;
      }
      else if (buf->readableBytes() >= len + kHeaderLen)
      {
        buf->retrieve(kHeaderLen);
        std::string message(buf->peek(), len);
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(len);
      }
      else
      {
        break;
      }
    }
  }

  // FIXME: TcpConnectionPtr
  void send(TcpConnection* conn,
            const std::string& message)
  {
    Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = sockets::hostToNetwork32(len);
    buf.prepend(&be32, sizeof be32);
	std::string msg(buf.peek(), buf.readableBytes());
    conn->send(msg);
  }

 private:
  StringMessageCallback messageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
