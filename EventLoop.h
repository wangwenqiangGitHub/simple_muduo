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
#include "Callbacks.h"
#include "TimerQueue.h"

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
    void runInLoop(const Functor& func);
    // 把上层注册的回调函数cb放入队列中 唤醒loop所在的线程执行cb
    void queueInLoop(const Functor& cb);

	// 定时器
    TimerId addTimer(const TimerCallback& cb, const Timestamp& when);
    TimerId addTimer(const TimerCallback& cb, double delaySeconds, bool repeat = false);
    void    cancelTimer(TimerId id);

    bool isRunning() { return running_; }
    void assertInLoopThread() const;

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
	std::atomic_bool running_;

	std::vector<Functor> pendingFunctors_;
	std::mutex mutex_;
    TimerQueue               *timerQueue_;
};
}

#endif //SIMPLE_MUDUO_EVENTLOOP_H
