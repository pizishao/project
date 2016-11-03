#include "Epoller.h"
#include "base/Logger.h"
#include "EventLoop.h"

namespace MuduoPlus
{
    Epoller::Epoller(EventLoop* loop)
        : Poller(loop),
          epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
          events_(kInitEventListSize)
    {
        if(epollfd_ < 0)
        {
            LOG_PRINT(LogType_Fatal, "create epollfd failed");
        }
    }

    Epoller::~Epoller()
    {
        SocketOps::closeSocket(epollfd_);
    }

    void Epoller::poll(int timeoutMs, ChannelHolderList &activeChannelHolders)
    {
        LOG_PRINT(LogType_Info, "fd total count %u", channelHolders_.size());

        int numEvents = ::epoll_wait(epollfd_, &events_[0],
                                     static_cast<int>(events_.size()),
                                     timeoutMs);

        int errorCode = GetLastErrorCode();

        if(numEvents > 0)
        {
            LOG_PRINT(LogType_Info, "epoll_wait %u events happended", numEvents);
            fillActiveChannelHolders(numEvents, activeChannelHolders);

            if((size_t)numEvents == events_.size())
            {
                events_.resize(events_.size() * 2);
            }
        }
        else if(numEvents == 0)
        {
            LOG_PRINT(LogType_Info, "epoll_wait nothing happended");
        }
        else
        {
            // error happens, log uncommon ones
            if(errorCode != EINTR)
            {
                LOG_PRINT(LogType_Error, "epoll_wait failed");
            }
        }
    }

    void Epoller::updateChannel(Channel* pChannel)
    {
        Poller::assertInLoopThread();

        if(hasChannel(pChannel))
        {
            epoll_event event;
            event.data.fd = pChannel->fd();
            event.events = pChannel->getEpEvents();

            if(epoll_ctl(epollfd_, EPOLL_CTL_MOD, pChannel->fd(), &event) < 0)
            {
                LOG_PRINT(LogType_Error, "EPOLL_CTL_MOD failed:%s", GetLastErrorText().c_str());
            }
        }
        else
        {
            epoll_event event;
            event.data.fd = pChannel->fd();
            event.events = pChannel->getEpEvents();

            if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, pChannel->fd(), &event) < 0)
            {
                LOG_PRINT(LogType_Error, "EPOLL_CTL_ADD failed:%s", GetLastErrorText().c_str());
                return;
            }

            auto weakOwner = pChannel->getOwner();
            auto owner = weakOwner.lock();
            assert(owner);

            ChannelHolder holder;
            holder.channel_ = pChannel;
            holder.ower_ = owner;
            channelHolders_.insert({ pChannel->fd(), holder });
        }
    }

    void Epoller::removeChannel(Channel* pChannel)
    {
        Poller::assertInLoopThread();

        auto count = channelHolders_.erase(pChannel->fd());
        assert(count > 0);

        epoll_event event;
        event.data.fd = pChannel->fd();
        event.events = pChannel->getEpEvents();

        if(epoll_ctl(epollfd_, EPOLL_CTL_DEL, pChannel->fd(), &event) < 0)
        {
            LOG_PRINT(LogType_Error, "EPOLL_CTL_DEL failed %s", GetLastErrorText().c_str());
        }
    }

    void Epoller::fillActiveChannelHolders(int numEvents, ChannelHolderList
                                           &activeChannelHolders) const
    {
        for(int i = 0; i < numEvents; i++)
        {
            socket_t fd = events_[i].data.fd;
            auto it = channelHolders_.find(fd);

            if(it != channelHolders_.end())
            {
                ChannelHolder holder = it->second;
                Channel *pChannel = holder.channel_;

                int recvEvents = Channel::kNoneEvent;
                auto epEvents = events_[i].events;

                if(epEvents & (POLLIN | POLLPRI | POLLRDHUP))
                {
                    recvEvents |= Channel::kReadEvent;
                }

                if(epEvents & POLLOUT)
                {
                    recvEvents |= Channel::kWriteEvent;
                }

                if(epEvents & (POLLERR | POLLHUP))
                {
                    recvEvents |= Channel::kErrorEvent;
                }

                if(recvEvents)
                {
                    LOG_PRINT(LogType_Debug, "recvEvents:%d", recvEvents);
                    pChannel->setRecvEvents(recvEvents);
                    activeChannelHolders.push_back(holder);
                }
                else
                {
                    LOG_PRINT(LogType_Debug, "none wait events");
                }
            }
            else
            {
                LOG_PRINT(LogType_Error, "can not find fd[%d] associate channel", fd);
            }
        }
    }
}