#ifndef SIMPLE_MUDUO_CALLBACKS_H
#define SIMPLE_MUDUO_CALLBACKS_H
#include <memory>
#include <functional>

namespace simple_muduo{
class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);

using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;

using TimerId = int;

using TimerCallback = std::function<void ()>;
using SignalCallback = std::function<void (int)>;
}
#endif
