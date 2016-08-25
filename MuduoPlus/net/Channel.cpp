#include <assert.h>

#include "base/Timestamp.h"
#include "Channel.h"
#include "EventLoop.h"

namespace MuduoPlus
{
    const int Channel::kNoneEvent = 0x00;
    const int Channel::kReadEvent = 0x01;
    const int Channel::kWriteEvent = 0x02;
    const int Channel::kErrorEvent = 0x04;

    Channel::Channel(EventLoop* loop, int fd)
        : loop_(loop),
        fd_(fd),
        interestEvents_(kNoneEvent),
        trigerEvents_(kNoneEvent)
    {
    }

    Channel::~Channel()
    {
        if (loop_->isInLoopThread())
        {
            assert(!loop_->hasChannel(this));
        }
    }

    void Channel::update()
    {
        addedToLoop_ = true;
        loop_->updateChannel(this);
    }

    void Channel::remove()
    {
        assert(isNoneEvent());
        addedToLoop_ = false;
        loop_->removeChannel(this);
    }

    void Channel::setOwner(std::weak_ptr<void> ptr)
    {
        owner_ = ptr;
    }

    std::weak_ptr<void> Channel::getOwner()
    {
        return owner_;
    }

#ifndef WIN32
    int  Channel::getEpEvents()
    {
        int events = 0;

        if (interestEvents_ & Channel::kReadEvent)
        {
            events |= POLLIN;
        }

        if (interestEvents_ & Channel::kWriteEvent)
        {
            events |= POLLOUT;
        }

        if (interestEvents_ & Channel::kErrorEvent)
        {
            events |= EPOLLRDHUP; /*  epoll_wait will always wait for EPOLLERR¡¢EPOLLHUP event */
        }

        return events;
    }
#endif    

    void Channel::handleEvent(Timestamp receiveTime)
    {
        handleEventWithGuard(receiveTime);
    }

    void Channel::handleEventWithGuard(Timestamp receiveTime)
    {
        if (trigerEvents_ & kReadEvent)
        {
            if (readCallback_)
            {
                readCallback_(receiveTime);
            }
        }

        if (trigerEvents_ & kWriteEvent)
        {
            if (writeCallback_)
            {
                writeCallback_();
            }
        }

        if (trigerEvents_ & kErrorEvent)
        {
            if (closeCallback_)
            {
                closeCallback_();
            }
        }

        if (finishCallback_)
        {
            finishCallback_();
        }
    }
}