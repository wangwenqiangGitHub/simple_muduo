//=====================================================================
//
// TimerQueue.cpp - 
//
// Created by wwq on 2022/08/23
// Last Modified: 2022/08/23 22:05:09
//
//=====================================================================
#include "TimerQueue.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "Timer.h"
#include <cassert>
#include <utility>
namespace simple_muduo{
TimerQueue::TimerQueue(EventLoop* loop)
	: loop_(loop)
	, callingTimesFunctor_(false)
{
	
}
TimerQueue::~TimerQueue()
{

}
TimerId TimerQueue::addTimer(const TimerCallback& cb, const Timestamp& when, double interval)
{
	TimerId id = ++atomic_;
	Timer* timer = new Timer(id, cb, when, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this, timer));
	return id;
}

void TimerQueue::cancelTimer(TimerId id)
{
	loop_->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, id));
}

Timestamp TimerQueue::getNearestExpiration() const
{
     if(timers_.empty())
         return Timestamp::invalid();
     return timers_.begin()->first;
}

void TimerQueue::cancelTimerInLoop(TimerId id)
{
	loop_->assertInLoopThread();
	TimerMap::iterator it = activeTimers_.find(id);
	if(it != activeTimers_.end())
	{
		TimerList::iterator iter = timers_.find(std::make_pair(it->second->expires_at(), it->second));
		assert(iter!= timers_.end());
		Timer* timer = it->second;
		delete timer;
		activeTimers_.erase(it);
		timers_.erase(iter);
	}
	else
	{
		if(callingTimesFunctor_)
		{
			cancelTimers_.insert(id);
		}
	}
	assert(timers_.size() == activeTimers_.size());
}
void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();
	addTimer(timer);
}
void TimerQueue::addTimer(Timer* timer)
{
	timers_.insert(std::make_pair(timer->expires_at(), timer));
	activeTimers_.insert(std::make_pair(timer->id(), timer));
}

void TimerQueue::runTimer(const Timestamp& now)
{
    if(timers_.empty())
        return;

    callingTimesFunctor_ = true;
    cancelTimers_.clear();  // just for saving cancel timers when runTimer

    std::vector<Entry> expired = getExpiredTimers(now);

    for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end(); ++it)
    {
        it->second->trigger();
    }

    callingTimesFunctor_ = false;

    for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end(); ++it)
    {
        Timer *timer = it->second;
        if(timer->repeat() && cancelTimers_.find(timer->id()) == cancelTimers_.end())
        {
            timer->restart(now);
            addTimer(timer);
        }
        else          // timer just run once or has already being canceled
        {
            delete timer;
        }
    }

    expired.clear();
    assert(timers_.size() == activeTimers_.size());
}

std::vector<TimerQueue::Entry> TimerQueue::getExpiredTimers(const Timestamp& now)
{
    std::vector<Entry> expired;

    Entry piovt(now, reinterpret_cast<Timer*>(INT_MAX));
    TimerList::iterator end = timers_.lower_bound(piovt); // return the pos of that not less than piovt
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, std::back_inserter(expired));

    timers_.erase(timers_.begin(), end);
    for (std::vector< Entry >::iterator it = expired.begin(); it != expired.end(); ++it)
    {
        activeTimers_.erase(it->second->id());
    }

    // LOG_INFO("TimerQueue::getExpiredTimers [%d][%d][%d]", expired.size(), timers_.size(), activeTimers_.size());
    return expired;
}

} // namespace simple_muduo
