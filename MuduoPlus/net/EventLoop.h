#pragma once

#include <mutex>
#include <functional>
#include <vector>

#include "base/LinuxWin.h"
#include "base/NonCopyable.h"
#include "base/Timestamp.h"
#include "CallBack.h"
#include "TimerId.h"
#include "ChannelHolder.h"

namespace MuduoPlus
{
    class Channel;
    class Poller;
    class TimerQueue;

    class EventLoop : NonCopyable
    {
    public:
        typedef std::function<void()> Functor;

        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        Timestamp   pollReturnTime() const
        {
            return pollReturnTime_;
        }

        void runInLoop(const Functor& cb);
        void queueInLoop(const Functor& cb);

        TimerId runAt(const Timestamp& time, const TimerCallback& cb);
        TimerId runAfter(double delay, const TimerCallback& cb);
        TimerId runEvery(double interval, const TimerCallback& cb);
        void    cancel(TimerId timerId);
        void    resetPollTimeOut(int msec);

        bool IsPollReturn() const
        {
            return pollReturned;
        }
        void wakeup();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

        void assertInLoopThread()
        {
            if(!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const
        {
            return threadId_ == GetCurrentThreadId();
        }

        bool eventHandling() const
        {
            return eventHandling_;
        }

        /*void setContext(const boost::any& context)
        {
            context_ = context;
        }

        const boost::any& getContext() const
        {
            return context_;
        }

        boost::any* getMutableContext()
        {
            return &context_;
        }*/

    private:
        void abortNotInLoopThread();
        void checkTimeOut();
        void handleRead();
        void doPendingFunctors();

        void printActiveChannels() const;

        typedef std::vector<ChannelHolder>   ChannelHolderList;

        bool                        looping_;
        bool                        pollReturned;
        bool                        quit_;
        bool                        eventHandling_;
        bool                        callingPendingFunctors_;
        const int                   threadId_;
        int                         pollTimeoutMsec_;
        Timestamp                   timeOutCheckStamp_;
        Timestamp                   pollReturnTime_;
        std::shared_ptr<Poller>     poller_;
        std::shared_ptr<TimerQueue> timerQueue_;
        socket_t                    wakeupFdPair_[2];
        std::shared_ptr<Channel>    wakeupChannel_;
        //boost::any                  context_;

        ChannelHolderList           activeChannelHolders_;

        std::mutex                  mutex_;
        std::vector<Functor>        pendingFunctors_;
    };
}