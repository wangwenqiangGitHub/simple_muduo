//=====================================================================
//
// EventLoopThreadPool.h - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 18:12:12
//
//=====================================================================
#ifndef SIMPLE_MUDUO_EVENTLOOPTHREADPOOL_H
#define SIMPLE_MUDUO_EVENTLOOPTHREADPOOL_H
#include "EventLoopThread.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

namespace simple_muduo {
class EventLoop;
class EventLoopThread;

//EventLoopThreadPool是事件循环线程池
class EventLoopThreadPool : noncopyable
{
public:
	using ThreadInitCallback = std::function<void(EventLoop*)>;
	EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
	~EventLoopThreadPool();

	void setThreadNum(int numThreads) {numThreads_ = numThreads;}
	void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // 如果工作在多线程中
	// baseLoop_(mainLoop)会默认以轮询的方式分配Channel给subLoop
	EventLoop* getNextLoop();
	std::vector<EventLoop*> getAllLoops();
	bool started() const{return started_;}
	const std::string name() const {return name_;}
private:
	// 用户定义的eventLoop
	EventLoop* baseLoop_;
	std::string name_;
	bool started_;
	int numThreads_;
	int next_;
	std::vector<std::unique_ptr<EventLoopThread>> threads_;
	std::vector<EventLoop*> loops_;
};
}

#endif
