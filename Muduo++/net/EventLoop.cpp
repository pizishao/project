#include <limits.h>

#include "base/LinuxWin.h"

#include "EventLoop.h"
#include "Channel.h"
#include "TimerQueue.h"
#include "SocketOps.h"
#include "base/Logger.h"

#ifdef WIN32
#include "Selector.h"
#else
#include "Epoller.h"
#endif

namespace MuduoPlus
{

    EventLoop::EventLoop()
        : looping_(false),
        quit_(false),
        eventHandling_(false),
        callingPendingFunctors_(false),
        threadId_(GetCurThreadID()),        
        timerQueue_(new TimerQueue(this))     
    {
        /*LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
        if (t_loopInThisThread)
        {
            LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                << " exists in this thread " << threadId_;
        }
        else
        {
            t_loopInThisThread = this;
        }*/

#ifdef WIN32
        poller_.reset(new Selector(this));
#else
        poller_.reset(new Epoller(this));
#endif

        memset(wakeupFdPair_, 0, sizeof(wakeupFdPair_));
        SocketOps::createSocketPair(wakeupFdPair_);

        wakeupChannel_.reset(new Channel(this, wakeupFdPair_[1]));
        wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
        // we are always reading the wakeupfd
        wakeupChannel_->enableReading();

        pollTimeoutMsec_ = INT_MAX;
        sentryCheckTimeStamp_ = Timestamp::now();
    }

    EventLoop::~EventLoop()
    {
        /*LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << CurrentThread::tid();*/
        wakeupChannel_->disableAll();
        wakeupChannel_->remove();
        SocketOps::closeSocket(wakeupFdPair_[0]);
        SocketOps::closeSocket(wakeupFdPair_[1]);
        /*t_loopInThisThread = NULL;*/
    }

    void EventLoop::loop()
    {
        assert(!looping_);
        assertInLoopThread();
        looping_ = true;
        quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
        //LOG_TRACE << "EventLoop " << this << " start looping";

        while (!quit_)
        {
            activeChannelHolders_.clear();
            poller_->poll(pollTimeoutMsec_, activeChannelHolders_);
            /*if (Logger::logLevel() <= Logger::TRACE)
            {
                printActiveChannels();
            }*/
            // TODO sort channel by priority          

            eventHandling_ = true;
            /*for (ChannelHolderList::iterator it = activeChannelHolders_.begin();
                it != activeChannelHolders_.end(); ++it)*/
            pollReturnTime_ = Timestamp::now();
            for (auto &pos : activeChannelHolders_)
            {
                Channel *pChannel = pos.channel_;
                pChannel->handleEvent(pollReturnTime_);
            }

            eventHandling_ = false;
            doPendingFunctors();

            checkTimeOut();
        }

        //LOG_TRACE << "EventLoop " << this << " stop looping";
        looping_ = false;
    }

    void EventLoop::quit()
    {
        quit_ = true;
        // There is a chance that loop() just executes while(!quit_) and exits,
        // then EventLoop destructs, then we are accessing an invalid object.
        // Can be fixed using mutex_ in both places.
        if (!isInLoopThread())
        {
            wakeup();
        }
    }

    void EventLoop::runInLoop(const Functor& cb)
    {
        if (isInLoopThread())
        {
            cb();
        }
        else
        {
            queueInLoop(cb);
        }
    }

    void EventLoop::queueInLoop(const Functor& cb)
    {
        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            pendingFunctors_.push_back(cb);
        }

        if (!isInLoopThread() || callingPendingFunctors_)
        {
            wakeup();
        }
    }

    TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
    {
        return timerQueue_->addTimer(cb, time, 0.0);
    }

    TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
    {
        return runAt(Timestamp::now().addSeconds(delay), cb);
    }

    TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
    {
        return timerQueue_->addTimer(cb, Timestamp::now().addSeconds(interval), interval);
    }

    void EventLoop::cancel(TimerId timerId)
    {
        return timerQueue_->cancel(timerId);
    }

    void EventLoop::resetPollTimeOut(int msec)
    {
        assert(msec >= 0);

        pollTimeoutMsec_ = msec;
        sentryCheckTimeStamp_ = Timestamp::now();

        wakeup();
    }

    void EventLoop::updateChannel(Channel* channel)
    {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();

        poller_->updateChannel(channel);
    }

    void EventLoop::removeChannel(Channel* channel)
    {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();

        if (eventHandling_)
        {
#if DEBUG
            bool bFind = false;
            for (auto &pos : activeChannelHolders_)
            {
                if (pos.channel_ == channel)
                {
                    bFind = true;
                }
            }

            assert(bFind);
#endif
        }

        poller_->removeChannel(channel);
    }

    bool EventLoop::hasChannel(Channel* channel)
    {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();

        return poller_->hasChannel(channel);
    }

    void EventLoop::abortNotInLoopThread()
    {
        /*LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << CurrentThread::tid();*/
        LOG_PRINT(LogType_Fatal, "EventLoop::abortNotInLoopThread - EventLoop %p was created in "
            "threadId_ = %u, current thread id = %u", threadId_, GetCurThreadID());
    }

    void EventLoop::wakeup()
    {
        char buf[1] = { 0 };
        int r;

#ifdef WIN32
        r = send(wakeupFdPair_[0], buf, 1, 0);
#else
        r = write(wakeupFdPair_[0], buf, 1);
#endif

        if (r < 0 && GetLastErrorCode() != EAGAIN)
        {
            assert(false);
        }
    }

    void EventLoop::checkTimeOut()
    {
        Timestamp nowStamp = Timestamp::now();
        long durationMsec = millisecondDifference(nowStamp, sentryCheckTimeStamp_);

        if ((long)durationMsec >= pollTimeoutMsec_)
        {
            timerQueue_->timeOut();
        }

        sentryCheckTimeStamp_ = nowStamp;
        pollTimeoutMsec_ -= durationMsec;
    }

    void EventLoop::handleRead()
    {
        unsigned char buf[1024] = { 0 };
#ifdef WIN32
        while (recv(wakeupFdPair_[1], (char*)buf, sizeof(buf), 0) > 0)
            ;
#else
        while (read(wakeupFdPair_[1], (char*)buf, sizeof(buf)) > 0)
            ;
#endif
    }

    void EventLoop::doPendingFunctors()
    {
        std::vector<Functor> functors;
        callingPendingFunctors_ = true;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            functors.swap(pendingFunctors_);
        }

        for (size_t i = 0; i < functors.size(); i++)
        {
            functors[i]();
        }

        callingPendingFunctors_ = false;
    }

    void EventLoop::printActiveChannels() const
    {
        for (ChannelHolderList::const_iterator it = activeChannelHolders_.begin();
            it != activeChannelHolders_.end(); ++it)
        {
            const Channel* ch = it->channel_;
            //LOG_TRACE << "{" << ch->reventsToString() << "} ";
        }
    }
}