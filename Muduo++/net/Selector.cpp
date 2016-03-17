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
        FD_ZERO(&readfds_);
        FD_ZERO(&writefds_);
        FD_ZERO(&exceptfds_);
    }

    Selector::~Selector()
    {
    }

    void Selector::poll(int timeoutMS, PipeList* activePipes)
    {
        resetFDSet();

        timeval tv = {0};

        long sec = timeoutMS / 1000;
        long msec = timeoutMS - sec * 1000;

        tv.tv_sec = sec;
        tv.tv_usec = msec * 1000;

        // zero if the time limit expired, or SOCKET_ERROR if an error occurred
        int iRet = select(0, &readfds_, &writefds_, &exceptfds_, &tv); 

        if (iRet <= 0)
        {
            if (iRet < 0)
            {
                assert(false);
            }

            return;
        }

        fillActivePipes(activePipes);
    }

    void Selector::fillActivePipes(PipeList* activePipes) const
    {
        for (const auto& pipePair : pipes_)
        {
            Pipe    pipe = pipePair.second;
            Channel *pChannel = pipe.channel_;
            int events = Channel::kNoneEvent;

            if (FD_ISSET(pChannel->fd(), &readfds_))
            {
                events |= Channel::kReadEvent;
            }

            if (FD_ISSET(pChannel->fd(), &writefds_))
            {
                events |= Channel::kWriteEvent;
            }

            if (FD_ISSET(pChannel->fd(), &exceptfds_))
            {
                events |= Channel::kCloseEvent;
            }

            if (events)
            {
                pChannel->setRecvEvents(events);
                activePipes->push_back(pipe);
            }
        }
    }

    void Selector::updateChannel(Channel* channel)
    {
        if (pipes_.find(channel->fd()) == pipes_.end())
        {
            auto weakOwner = channel->getOwner();
            auto owner = weakOwner.lock();
            if (owner)
            {
                Pipe pipe;
                pipe.channel_ = channel;
                pipe.ower_ = owner;
                pipes_.insert({ channel->fd(), pipe});
            }
            else
            {
                assert(false);
            }
        } 
        else
        {
            // do nothing
        }
    }

    void Selector::removeChannel(Channel* channel)
    {
        Poller::assertInLoopThread();

        int fd = channel->fd();

#if DEBUG
        auto found = pipes_.find(fd);
        assert(found != pipes_.end());
        assert(found->second.channel_ == channel);
#endif       

        pipes_.erase(fd);
    }

    void Selector::resetFDSet()
    {
        FD_ZERO(&readfds_);
        FD_ZERO(&writefds_);
        FD_ZERO(&exceptfds_);

        for (const auto& pipePair : pipes_)
        {
            Channel *pChannel = pipePair.second.channel_;

            if (pChannel->interestEvents() & Channel::kReadEvent)
            {
                FD_SET(pChannel->fd(), &readfds_);
            }

            if (pChannel->interestEvents() & Channel::kWriteEvent)
            {
                FD_SET(pChannel->fd(), &writefds_);
            }

            if (pChannel->interestEvents() & Channel::kCloseEvent)
            {
                FD_SET(pChannel->fd(), &exceptfds_);
            }
        }
    }
}