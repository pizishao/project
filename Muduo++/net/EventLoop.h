#pragma once

#include <mutex>
#include <functional>
#include <vector>

#include "base/LinuxWin.h"
#include "base/NonCopyable.h"
#include "base/Timestamp.h"
#include "CallBack.h"
#include "TimerId.h"

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

        Timestamp   pollReturnTime() const { return pollReturnTime_; }
        int64_t     iteration() const { return iteration_; }

        void runInLoop(const Functor& cb);
        void queueInLoop(const Functor& cb);

        TimerId runAt(const Timestamp& time, const TimerCallback& cb);
        TimerId runAfter(double delay, const TimerCallback& cb);
        TimerId runEvery(double interval, const TimerCallback& cb);
        void    cancel(TimerId timerId);

        void wakeup();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const { return threadId_ == GetCurThreadID(); }

        bool eventHandling() const { return eventHandling_; }

        void setContext(const boost::any& context)
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
        }

    private:
        void abortNotInLoopThread();
        void handleRead();
        void doPendingFunctors();

        void printActiveChannels() const;

        typedef std::vector<Channel*> ChannelList;

        bool                        looping_; 
        bool                        quit_; 
        bool                        eventHandling_; 
        bool                        callingPendingFunctors_; 
        int64_t                     iteration_;
        const int                   threadId_;
        int                         m_PollTimeOutMs;
        int                         m_AccumulateTimedOutMs;
        Timestamp                   pollReturnTime_;
        std::shared_ptr<Poller>     poller_;
        std::shared_ptr<TimerQueue> timerQueue_;
        int wakeupFd_;
        std::shared_ptr<Channel>    wakeupChannel_;
        boost::any                  context_;

        ChannelList                 activeChannels_;
        Channel*                    currentActiveChannel_;

        std::mutex                  mutex_;
        std::vector<Functor>        pendingFunctors_;
    };
}