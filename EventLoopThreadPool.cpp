//=====================================================================
//
// EventLoopThreadPool.cpp - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 18:30:19
//
//=====================================================================
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include <cstdio>

using namespace simple_muduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg)
{

}
EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
	started_ = true;
	for(int i = 0; i < numThreads_; i++)
	{
		char buf[name_.size() + 32];
		snprintf(buf, sizeof(buf), "%s%d", name_.c_str(),i);
		EventLoopThread *t = new EventLoopThread(cb,buf);
		loops_.push_back(t->startLoop());
	}

	if(numThreads_ == 0 && cb)
	{
		cb(baseLoop_);
	}
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	EventLoop *loop = baseLoop_;
	if(!loops_.empty())
	{
		loop = loops_[next_];
		++next_;
		if(next_ >= loops_.size())
		{
			next_ = 0;
		}
	}
	return loop;
}


std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
	if(!loops_.empty())
	{
		return std::vector<EventLoop*>(1, baseLoop_);
	}
	else
	{
		return loops_;
	}
}
