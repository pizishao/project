#include <WinSock2.h>
#include <assert.h>

#include "Selector.h"
#include "Poller.h"
#include "Channel.h"

namespace MuduoPlus
{
    Selector::Selector(EventLoop* loop)
        : Poller(loop)
    {
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&exceptSet);
    }

    Selector::~Selector()
    {
    }

    void Selector::poll(int timeoutMs, ChannelList* activeChannels)
    {
        resetFDSet();

        timeval tv = {0};

        long sec = timeoutMs / 1000;
        long odd_msec = timeoutMs - sec * 1000;

        tv.tv_sec = sec;
        tv.tv_usec = odd_msec * 1000;

        int iRet = select(0, &readSet, &writeSet, &exceptSet, &tv);

        if (iRet <= 0)
        {
            return;
        }

        fillActiveChannels(activeChannels);
    }

    void Selector::fillActiveChannels(ChannelList* activeChannels) const
    {
        for (const auto& channelPair : channels_)
        {
            Channel *pChannel = channelPair.second;
            int events = Channel::kNoneEvent;

            if (FD_ISSET(pChannel->fd(), &readSet))
            {
                events |= Channel::kReadEvent;
            }

            if (FD_ISSET(pChannel->fd(), &writeSet))
            {
                events |= Channel::kWriteEvent;
            }

            if (FD_ISSET(pChannel->fd(), &exceptSet))
            {
                events |= Channel::kErrorEvent;
            }

            if (events)
            {
                pChannel->set_revents(events);
                activeChannels->push_back(pChannel);
            }
        }
    }

    void Selector::updateChannel(Channel* channel)
    {
        // do nothing
    }

    void Selector::removeChannel(Channel* channel)
    {
        Poller::assertInLoopThread();

        int fd = channel->fd();
        auto found = channels_.find(fd);
        assert(found != channels_.end());
        assert(found->second == channel);

        channels_.erase(fd);
    }

    void Selector::resetFDSet()
    {
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&exceptSet);

        for (const auto& channelPair : channels_)
        {
            Channel *pChannel = channelPair.second;

            if (pChannel->events() & Channel::kReadEvent)
            {
                FD_SET(pChannel->fd(), &readSet);
            }

            if (pChannel->events() & Channel::kWriteEvent)
            {
                FD_SET(pChannel->fd(), &writeSet);
            }

            if (pChannel->events() & Channel::kErrorEvent)
            {
                FD_SET(pChannel->fd(), &exceptSet);
            }
        }
    }
}