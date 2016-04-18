#include <WinSock2.h>
#include <assert.h>

#include "Selector.h"
#include "Poller.h"
#include "Channel.h"
#include "base/Logger.h"

namespace MuduoPlus
{
    Selector::Selector(EventLoop* loop)
        : Poller(loop), loop_(loop)
    {
        FD_ZERO(&readFds_);
        FD_ZERO(&writeFds_);
        FD_ZERO(&exceptFds_);
    }

    Selector::~Selector()
    {
    }

    void Selector::poll(int timeoutMS, ChannelHolderList &activeChannelHolders)
    {
        resetFdSet();

        timeval tv = {0};

        long sec = timeoutMS / 1000;
        long msec = timeoutMS - sec * 1000;

        tv.tv_sec = sec;
        tv.tv_usec = msec * 1000;

        // zero if the time limit expired, or SOCKET_ERROR if an error occurred
        int iRet = select(0, &readFds_, &writeFds_, &exceptFds_, &tv); 

        if (iRet <= 0)
        {
            if (iRet < 0)
            {
                LOG_PRINT(LogType_Fatal, "select failed:%s", GetLastErrorText().c_str());
            }

            return;
        }

        fillActiveChannelHolders(activeChannelHolders);
    }

    void Selector::fillActiveChannelHolders(ChannelHolderList &activeChannelHolders) const
    {
        for (const auto& pos : channelHolders_)
        {
            ChannelHolder    holder = pos.second;
            Channel *pChannel = holder.channel_;
            int events = Channel::kNoneEvent;

            if (FD_ISSET(pChannel->fd(), &readFds_))
            {
                events |= Channel::kReadEvent;
            }

            if (FD_ISSET(pChannel->fd(), &writeFds_))
            {
                events |= Channel::kWriteEvent;
            }

            if (FD_ISSET(pChannel->fd(), &exceptFds_))
            {
                events |= Channel::kErrorEvent;
            }

            if (events)
            {
                pChannel->setRecvEvents(events);
                activeChannelHolders.push_back(holder);
            }
        }
    }

    void Selector::updateChannel(Channel* channel)
    {
        if (channelHolders_.find(channel->fd()) == channelHolders_.end())
        {
            auto weakOwner = channel->getOwner();
            auto owner = weakOwner.lock();
            //if (owner)
            {
                ChannelHolder holder;
                holder.channel_ = channel;
                holder.ower_ = owner;
                channelHolders_.insert({ channel->fd(), holder});
            }
            /*else
            {
                assert(false);
            }*/
        } 
        else
        {
            // do nothing
        }

        //loop_->wakeup();
    }

    void Selector::removeChannel(Channel* channel)
    {
        Poller::assertInLoopThread();

        int fd = channel->fd();

#if DEBUG
        auto found = channelHolders_.find(fd);
        assert(found != channelHolders_.end());
        assert(found->second.channel_ == channel);
#endif       

        channelHolders_.erase(fd);
    }

    void Selector::resetFdSet()
    {
        FD_ZERO(&readFds_);
        FD_ZERO(&writeFds_);
        FD_ZERO(&exceptFds_);

        for (const auto& pos : channelHolders_)
        {
            Channel *pChannel = pos.second.channel_;

            if (pChannel->interestEvents() & Channel::kReadEvent)
            {
                FD_SET(pChannel->fd(), &readFds_);
            }

            if (pChannel->interestEvents() & Channel::kWriteEvent)
            {
                FD_SET(pChannel->fd(), &writeFds_);
            }

            if (pChannel->interestEvents() & Channel::kErrorEvent)
            {
                FD_SET(pChannel->fd(), &exceptFds_);
            }
        }
    }
}