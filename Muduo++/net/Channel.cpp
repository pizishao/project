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
        interestEvents_(0),
        recvEvents_(0),
        index_(-1)
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

    void Channel::handleEvent(Timestamp receiveTime)
    {
        handleEventWithGuard(receiveTime);
    }

    void Channel::handleEventWithGuard(Timestamp receiveTime)
    {
        if (recvEvents_ & kReadEvent)
        {
            if (readCallback_)
            {
                readCallback_(receiveTime);
            }
        }

        if (recvEvents_ & kWriteEvent)
        {
            if (writeCallback_)
            {
                writeCallback_();
            }
        }

        if (recvEvents_ & kErrorEvent)
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