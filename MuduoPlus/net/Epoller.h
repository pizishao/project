#pragma once

#include "Poller.h"
#include "Channel.h"

namespace MuduoPlus
{
    class Epoller : public Poller
    {
    public:
        Epoller(EventLoop* loop);
        virtual ~Epoller();

        virtual void poll(int timeoutMs, ChannelHolderList &activeChannelHolders);
        virtual void updateChannel(Channel* pChannel);
        virtual void removeChannel(Channel* pChannel);

    private:
        static const int kInitEventListSize = 16;

        void fillActiveChannelHolders(int numEvents, ChannelHolderList 
            &activeChannelHolders) const;

        typedef std::vector<struct epoll_event> EventList;

        socket_t    epollfd_;
        EventList   events_;
    };
}