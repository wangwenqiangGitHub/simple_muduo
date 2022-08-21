//=====================================================================
//
// EventLoopThread.cpp - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 16:24:42
//
//=====================================================================
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "thread.h"

using namespace simple_muduo;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
							     const std::string &name)
	: loop_(nullptr)
	, exiting_(false)
	, thread_(std::bind(&EventLoopThread::threadFunc, this), name)
	, mutex_()
	, cond_()
	, callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	if(loop_!= nullptr)
	{
		loop_->quit();
		thread_.join();
	}
}

EventLoop* EventLoopThread::startLoop()
{
	// 启用底层线程Thread类对象thread_中通过start()创建一个独立的EventLoop对象
	thread_.start();
	EventLoop* loop = nullptr;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while(loop_ == nullptr)
		{
			cond_.wait(lock);
		}
		loop = loop_;
	}
	return loop;
}

// 新建立一个EventLoopThread用这个方法可以新建一个eventLoop
void EventLoopThread::threadFunc()
{
	// 创建一个独立的EventLoop对象
	// 和上面的线程是一一对应的 级one loop per thread
    EventLoop loop; 

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
	// 执行EventLoop的loop() 开启了底层的Poller的poll()
    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;

}
