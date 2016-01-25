#include <WinSock2.h>
#include <assert.h>

#include "Selector.h"
#include "Poller.h"
#include "Channel.h"

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

Timestamp Selector::poll(int timeoutMs, ChannelList* activeChannels)
{
    resetFDSet();
    
    int iRet = select(0,&readSet,&writeSet,&exceptSet,nullptr);

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
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    channels_.erase(fd);
}

void Selector::resetFDSet()
{
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&exceptSet);

    for (const auto& channelPair: channels_)
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

/*
const char* Selector::operationToString(int op)
{
    switch (op)
    {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_DEL:
        return "DEL";
    case EPOLL_CTL_MOD:
        return "MOD";
    default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}*/