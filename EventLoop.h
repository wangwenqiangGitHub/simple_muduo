#ifndef SIMPLE_MUDUO_EVENTLOOP_H
#define SIMPLE_MUDUO_EVENTLOOP_H
#include <functional>
#include <sched.h>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

namespace simple_muduo{
class Channel;
class Poller;
class EventLoop : noncopyable
{
public:
	using Functor = std::function<void()>;

	EventLoop();
	~EventLoop();

	void loop();
	void quit();
	Timestamp pollReturnTime() const {return pollRetureTime_;}

    // 在当前loop中执行
    void runInLoop(Functor cb);
    // 把上层注册的回调函数cb放入队列中 唤醒loop所在的线程执行cb
    void queueInLoop(Functor cb);

	// 通过eventfd唤醒loop所在的线程
	void wakeup();

	// EventLoop调用Poller方法
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);

	bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}
private:
	void handleRead();
	void doPendingFunctors();
private:
	using ChanneList = std::vector<Channel*>;
	std::atomic_bool looping_;
	std::atomic_bool quit_;
	
	Timestamp pollRetureTime_;
    std::unique_ptr<Poller> poller_;
	const pid_t threadId_;

int wakeupFd_;
	std::unique_ptr<Channel> wakeupChannel_;

	ChanneList activeChannels_;

	std::atomic_bool callingPendingFunctors_;
	std::vector<Functor> pendingFunctors_;
	std::mutex mutex_;
};
}

#endif //SIMPLE_MUDUO_EVENTLOOP_H
