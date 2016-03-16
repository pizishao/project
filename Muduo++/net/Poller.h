#pragma once

#include <vector>
#include <map>

#include "EventLoop.h"
#include "Pipe.h"

namespace MuduoPlus
{
    class Channel;

    class Poller
    {
    public:
        typedef std::vector<Pipe> PipeList;

        Poller(EventLoop* loop);
        virtual ~Poller();

        virtual void poll(int timeoutMs, PipeList* activeChannels) = 0;

        virtual void updateChannel(Channel* channel) = 0;

        virtual void removeChannel(Channel* channel) = 0;

        virtual bool hasChannel(Channel* channel) const;

        static Poller* newDefaultPoller(EventLoop* loop);

        void assertInLoopThread() const
        {
            //ownerLoop_->assertInLoopThread();
        }

    protected:
        typedef std::map<int, Pipe> PipeMap;
        PipeMap pipes_;

    private:
        EventLoop* ownerLoop_;
    };
}