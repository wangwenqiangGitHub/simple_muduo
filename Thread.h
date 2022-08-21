//=====================================================================
//
// Thread.h - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 15:27:17
//
//=====================================================================
#ifndef SIMPLE_MUDUO_THREAD_H
#define SIMPLE_MUDUO_THREAD_H
#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <thread>

#include "noncopyable.h"
namespace simple_muduo {

class Thread : noncopyable
{
public:
	using ThreadFunc = std::function<void()>;

	explicit Thread(ThreadFunc, const std::string name = std::string());
	~Thread();

	void start();
	void join();

	bool started() {return started_;}
	pid_t tid() const {return  tid_;}
private:
	void setDefaultName();
private:
	bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;       // 在线程创建时再绑定
    ThreadFunc func_; // 线程回调函数
    std::string name_;
    static std::atomic_int numCreated_;
};
}
#endif
