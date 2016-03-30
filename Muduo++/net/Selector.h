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

        void resetFDSet();
        void fillActiveChannelHolders(ChannelHolderList &activeChannelHolders) const;

        FD_SET readfds_;
        FD_SET writefds_;
        FD_SET exceptfds_;
    };
}