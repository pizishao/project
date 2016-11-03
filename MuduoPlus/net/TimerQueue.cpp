#include <limits.h>

#include "TimerQueue.h"
#include "EventLoop.h"

namespace MuduoPlus
{
    TimerQueue::TimerQueue(EventLoop* loop)
        : loop_(loop),
          timers_(),
          callingExpiredTimers_(false)
    {
    }

    TimerQueue::~TimerQueue()
    {
        // do not remove channel, since we're in EventLoop::dtor();

        for(auto &pos : timers_)
        {
            delete pos.second;
        }
    }

    TimerId TimerQueue::addTimer(const TimerCallback& cb,
                                 Timestamp when,
                                 double interval)
    {
        Timer* pTimer = new Timer(cb, when, interval);

        loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,
                                   this, pTimer));

        return TimerId(pTimer, pTimer->sequence());
    }

    void TimerQueue::cancel(TimerId timerId)
    {
        loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop,
                                   this, timerId));
    }

    void TimerQueue::addTimerInLoop(Timer* timer)
    {
        loop_->assertInLoopThread();
        bool earliestChanged = insert(timer);

        if(earliestChanged)
        {
            Timestamp stamp = timer->expiration();

            if((int64_t)stamp.milliSecondFromNow() <= INT_MAX)
            {
                loop_->resetPollTimeOut((int)stamp.milliSecondFromNow());
            }
            else
            {
                loop_->resetPollTimeOut(INT_MAX);
            }
        }
    }

    void TimerQueue::cancelInLoop(TimerId timerId)
    {
        assert(timers_.size() == activeTimers_.size());
        loop_->assertInLoopThread();

        ActiveTimer timer(timerId.timer_, timerId.sequence_);
        ActiveTimerSet::iterator it = activeTimers_.find(timer);

        if(it != activeTimers_.end())
        {
            size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
            assert(n == 1);
            (void)n;
            delete it->first; // FIXME: no delete please
            activeTimers_.erase(it);
        }
        else if(callingExpiredTimers_)
        {
            cancelingTimers_.insert(timer);
        }

        assert(timers_.size() == activeTimers_.size());
    }

    void TimerQueue::timeOut()
    {
        loop_->assertInLoopThread();
        Timestamp now(Timestamp::now());

        std::vector<Entry> expired = getExpired(now);

        callingExpiredTimers_ = true;
        cancelingTimers_.clear();

        // safe to callback outside critical section
        for(std::vector<Entry>::iterator it = expired.begin();
                it != expired.end(); ++it)
        {
            it->second->run();
        }

        callingExpiredTimers_ = false;

        reset(expired, now);
    }

    std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
    {
        assert(timers_.size() == activeTimers_.size());

        std::vector<Entry> expired;
        Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
        TimerList::iterator end = timers_.lower_bound(sentry);

        assert(end == timers_.end() || now < end->first);

        std::copy(timers_.begin(), end, back_inserter(expired));
        timers_.erase(timers_.begin(), end);

        for(std::vector<Entry>::iterator it = expired.begin();
                it != expired.end(); ++it)
        {
            ActiveTimer timer(it->second, it->second->sequence());
            size_t n = activeTimers_.erase(timer);
            assert(n == 1);
            (void)n;
        }

        assert(timers_.size() == activeTimers_.size());

        return expired;
    }

    void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
    {
        Timestamp nextExpire;

        for(std::vector<Entry>::const_iterator it = expired.begin();
                it != expired.end(); ++it)
        {
            ActiveTimer timer(it->second, it->second->sequence());

            if(it->second->repeat()
                    && cancelingTimers_.find(timer) == cancelingTimers_.end())
            {
                it->second->restart(now);
                insert(it->second);
            }
            else
            {
                // FIXME move to a free list
                delete it->second; // FIXME: no delete please
            }
        }

        if(!timers_.empty())
        {
            nextExpire = timers_.begin()->second->expiration();
        }

        if(nextExpire.valid())
        {
            if((int64_t)nextExpire.milliSecondFromNow() <= INT_MAX)
            {
                loop_->resetPollTimeOut((int)nextExpire.milliSecondFromNow());
            }
            else
            {
                loop_->resetPollTimeOut(INT_MAX);
            }
        }
        else
        {
            loop_->resetPollTimeOut(INT_MAX);
        }
    }

    bool TimerQueue::insert(Timer* timer)
    {
        loop_->assertInLoopThread();
        assert(timers_.size() == activeTimers_.size());
        bool earliestChanged = false;
        Timestamp when = timer->expiration();
        TimerList::iterator it = timers_.begin();

        if(it == timers_.end() || when < it->first)
        {
            earliestChanged = true;
        }

        {
            std::pair<TimerList::iterator, bool> result =
                timers_.insert(Entry(when, timer));
            assert(result.second);
            (void)result;
        }

        {
            std::pair<ActiveTimerSet::iterator, bool> result
                = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
            assert(result.second);
            (void)result;
        }

        assert(timers_.size() == activeTimers_.size());

        return earliestChanged;
    }
}