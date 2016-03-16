#pragma once

#include "Poller.h"

namespace MuduoPlus
{
    class Selector : public Poller
    {
    public:
        Selector(EventLoop* loop);
        virtual ~Selector();

        virtual void poll(int timeoutMs, PipeList* activePipes);
        virtual void updateChannel(Channel* channel);
        virtual void removeChannel(Channel* channel);

    private:
        static const int kInitEventListSize = 16;

        void resetFDSet();
        void fillActivePipes(PipeList* activePipes) const;

        //typedef std::vector<struct epoll_event> EventList;

        FD_SET readSet_;
        FD_SET writeSet_;
        FD_SET exceptSet_;
    };
}