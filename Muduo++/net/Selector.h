#pragma once

#include "Poller.h"

namespace MuduoPlus
{
    class Selector : public Poller
    {
    public:
        Selector(EventLoop* loop);
        virtual ~Selector();

        virtual void poll(int timeoutMs, ChannelHolderList &activeChannelHolders);
        virtual void updateChannel(Channel* channel);
        virtual void removeChannel(Channel* channel);

    private:
        static const int kInitEventListSize = 16;

        void resetFdSet();
        void fillActiveChannelHolders(ChannelHolderList &activeChannelHolders) const;

        FD_SET readFds_;
        FD_SET writeFds_;
        FD_SET exceptFds_;
        EventLoop *loop_;
    };
}