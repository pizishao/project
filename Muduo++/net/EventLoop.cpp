#include "base/LinuxWin.h"

#include "EventLoop.h"
#include "Channel.h"
#include "TimerQueue.h"
#include "SocketOps.h"
#include "Selector.h"

namespace MuduoPlus
{

    EventLoop::EventLoop()
        : looping_(false),
        quit_(false),
        eventHandling_(false),
        callingPendingFunctors_(false),
        iteration_(0),
        threadId_(GetCurThreadID()),        
        timerQueue_(new TimerQueue(this)),        
        currentActiveChannel_(NULL),
        m_PollTimeOutMs(0),
        m_AccumulateTimedOutMs(0)
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
        poller_(Poller::newDefaultPoller(this));
#endif

        memset(wakeupFd_, 0, sizeof(wakeupFd_));
        SocketOps::CreateSocketPair(wakeupFd_);
        wakeupChannel_.reset(new Channel(this, wakeupFd_[1]));

        wakeupChannel_->setReadCallback(
            std::bind(&EventLoop::handleRead, this));
        // we are always reading the wakeupfd
        wakeupChannel_->enableReading();
    }

    EventLoop::~EventLoop()
    {
        /*LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << CurrentThread::tid();*/
        wakeupChannel_->disableAll();
        wakeupChannel_->remove();
        /*::close(wakeupFd_);
        t_loopInThisThread = NULL;*/
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
            activeChannels_.clear();
            poller_->poll(m_PollTimeOutMs, &activeChannels_);
            ++iteration_;
            /*if (Logger::logLevel() <= Logger::TRACE)
            {
                printActiveChannels();
            }*/
            // TODO sort channel by priority
            eventHandling_ = true;
            for (ChannelList::iterator it = activeChannels_.begin();
                it != activeChannels_.end(); ++it)
            {
                currentActiveChannel_ = *it;
                currentActiveChannel_->handleEvent(pollReturnTime_);
            }
            currentActiveChannel_ = NULL;
            eventHandling_ = false;
            doPendingFunctors();
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
        Timestamp time(addSecondTime(Timestamp::now(), delay));
        return runAt(time, cb);
    }

    TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
    {
        Timestamp time(addSecondTime(Timestamp::now(), interval));
        return timerQueue_->addTimer(cb, time, interval);
    }

    void EventLoop::cancel(TimerId timerId)
    {
        return timerQueue_->cancel(timerId);
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
            assert(currentActiveChannel_ == channel ||
                std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
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
    }

    void EventLoop::wakeup()
    {
        /*uint64_t one = 1;
        ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
        if (n != sizeof one)
        {
            LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
        }*/

        char buf[1] = { 0 };
        int r;

#ifdef WIN32
        r = send(wakeupFd_[0], buf, 1, 0);
#else
        r = write(wakeupFd_[0], buf, 1);
#endif

        if (r < 0 && errno != EAGAIN)
        {
            assert(false);
        }
    }

    void EventLoop::handleRead()
    {
        unsigned char buf[1024] = {0};
#ifdef WIN32
        while (recv(wakeupFd_[1], (char*)buf, sizeof(buf), 0) > 0)
            ;
#else
        while (read(recv(wakeupFd_[1], (char*)buf, sizeof(buf)) > 0)
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

        for (size_t i = 0; i < functors.size(); ++i)
        {
            functors[i]();
        }
        callingPendingFunctors_ = false;
    }

    void EventLoop::printActiveChannels() const
    {
        for (ChannelList::const_iterator it = activeChannels_.begin();
            it != activeChannels_.end(); ++it)
        {
            const Channel* ch = *it;
            //LOG_TRACE << "{" << ch->reventsToString() << "} ";
        }
    }
}