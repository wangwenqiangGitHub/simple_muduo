//=====================================================================
//
// EventLoopThread.h - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 15:20:56
//
//=====================================================================
#ifndef SIMPLE_MUDUO_EVENTLOOPTHREAD_H
#define SIMPLE_MUDUO_EVENTLOOPTHREAD_H
#include "EventLoop.h"
#include "noncopyable.h"
#include "thread.h"

#include <condition_variable>
#include <functional>
#include <mutex>
namespace simple_muduo {

// muduo采用的是one loop per thread + threadpool(线程池)
// 任何一个线程具有eventloop 就称之为IO线程;
// EventLoopThread类就是封装了IO线程
class EventLoopThread : noncopyable
{
public:
	using ThreadInitCallback = std::function<void(EventLoop*)>;
	EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
			const std::string &name = std::string());
	~EventLoopThread();

	EventLoop *startLoop();
private:
	void threadFunc();

	EventLoop* loop_;
	bool exiting_;
	Thread thread_;
	std::mutex mutex_;
	std::condition_variable cond_;
	ThreadInitCallback callback_;
};
}
#endif
