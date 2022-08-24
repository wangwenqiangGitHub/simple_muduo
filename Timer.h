//=====================================================================
//
// Timer.h - 
//
// Created by wwq on 2022/08/23
// Last Modified: 2022/08/23 20:47:27
//
//=====================================================================
#ifndef SIMPLE_MUDUO_TIMER_H
#define SIMPLE_MUDUO_TIMER_H
#include "Callbacks.h"
#include "Timestamp.h"
namespace simple_muduo{

class EventLoop;
class Timer
{
public:
	Timer(TimerId id, const TimerCallback& cb, const Timestamp& when, double interval)
		: id_(id)
		, callback_(cb)
		, when_(when)
		, interval_(interval)
	{

	}

    TimerId  id() const { return id_; }

    Timestamp expires_at() const { return when_; }

    bool repeat() const { return interval_ > 0; }

    void trigger() const { callback_(); }

    void restart(const Timestamp& now)
    {
        if(repeat())
        {
            when_ = now + interval_;
        }
        else
        {
            when_ = Timestamp::invalid();
        }
    }
private:
    TimerId       id_;
    TimerCallback callback_;
    Timestamp     when_;
    double        interval_;  // second
};
}

#endif
