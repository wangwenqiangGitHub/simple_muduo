//=====================================================================
//
// TimerQueue.h - 
//
// Created by wwq on 2022/08/23
// Last Modified: 2022/08/23 21:51:54
//
//=====================================================================
#ifndef SIMPLE_MUDUO_TIMERQUEUE_H
#define SIMPLE_MUDUO_TIMERQUEUE_H
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <atomic>
#include <climits>

#include "Callbacks.h"
#include "Timestamp.h"
namespace simple_muduo{
class Timer;
class EventLoop;

class TimerQueue
{
	using Entry = std::pair<Timestamp, Timer*>;
	using TimerList = std::set<Entry>;
	using TimerMap = std::unordered_map<TimerId, Timer*>;
	using CancelTimerList = std::unordered_set<TimerId>;
public:
	explicit TimerQueue(EventLoop* loop);
	~TimerQueue();
public:
	TimerId addTimer(const TimerCallback& cb, const Timestamp& when, double interval);
	void cancelTimer(TimerId id);
    Timestamp getNearestExpiration() const;
    void    runTimer(const Timestamp& now);
private:
	void addTimerInLoop(Timer* timer);
    void cancelTimerInLoop(TimerId id);
    void addTimer(Timer* timer);
	std::vector<TimerQueue::Entry> getExpiredTimers(const Timestamp& now);
private:
    TimerList                timers_;
    TimerMap                 activeTimers_;
    CancelTimerList          cancelTimers_;

    EventLoop                *loop_;
	std::atomic_int atomic_;
	std::atomic_bool callingTimesFunctor_;
};
}
#endif //SIMPLE_MUDUO_TIMERQUEUE_H
