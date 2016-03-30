#pragma once

#include <vector>
#include <map>

#include "EventLoop.h"
#include "ChannelHolder.h"

namespace MuduoPlus
{
    class Channel;

    class Poller
    {
    public:
        typedef std::vector<ChannelHolder> ChannelHolderList;

        Poller(EventLoop* loop);
        virtual ~Poller();

        virtual void poll(int timeoutMs, ChannelHolderList &activeChannelHolders) = 0;

        virtual void updateChannel(Channel* channel) = 0;

        virtual void removeChannel(Channel* channel) = 0;

        virtual bool hasChannel(Channel* channel) const;

        static Poller* newDefaultPoller(EventLoop* loop);

        void assertInLoopThread() const
        {
            //ownerLoop_->assertInLoopThread();
        }

    protected:
        typedef std::map<int, ChannelHolder>    ChannelHolderMap;
        ChannelHolderMap                        channelHolders_;

    private:
        EventLoop* ownerLoop_;
    };
}