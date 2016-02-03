#pragma once

#include "Poller.h"

namespace MuduoPlus
{
    class Selector : public Poller
    {
    public:
        Selector(EventLoop* loop);
        virtual ~Selector();

        virtual void poll(int timeoutMs, ChannelList* activeChannels);
        virtual void updateChannel(Channel* channel);
        virtual void removeChannel(Channel* channel);

    private:
        static const int kInitEventListSize = 16;

        void resetFDSet();
        void fillActiveChannels(ChannelList* activeChannels) const;

        typedef std::vector<struct epoll_event> EventList;

        FD_SET readSet;
        FD_SET writeSet;
        FD_SET exceptSet;
    };
}