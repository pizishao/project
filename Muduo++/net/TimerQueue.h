#pragma once

#include <stdint.h>
#include <vector>
#include <set>

#include "base/NonCopyable.h"
#include "base/Timestamp.h"
#include "Timer.h"
#include "Channel.h"
#include "CallBack.h"

namespace MuduoPlus
{
    class EventLoop;
    class Timer;
    class TimerId;

    class TimerQueue : NonCopyable
    {
    public:
        TimerQueue(EventLoop* loop);
        ~TimerQueue();

        TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);
        void    cancel(TimerId timerId);
        void    timeOut();

    private:

        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;
        typedef std::pair<Timer*, int64_t> ActiveTimer;
        typedef std::set<ActiveTimer> ActiveTimerSet;

        void                addTimerInLoop(Timer* timer);
        void                cancelInLoop(TimerId timerId);        
        std::vector<Entry>  getExpired(Timestamp now);
        void                reset(const std::vector<Entry>& expired, Timestamp now);
        bool                insert(Timer* timer);

        EventLoop*      loop_;
        TimerList       timers_;

        ActiveTimerSet  activeTimers_;
        bool            callingExpiredTimers_;
        ActiveTimerSet  cancelingTimers_;
    };
}