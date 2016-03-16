#include "Poller.h"
#include "Channel.h"

namespace MuduoPlus
{

    Poller::Poller(EventLoop* loop)
        : ownerLoop_(loop)
    {
    }

    Poller::~Poller()
    {
    }

    bool Poller::hasChannel(Channel* channel) const
    {
        assertInLoopThread();
        PipeMap::const_iterator it = pipes_.find(channel->fd());
        return it != pipes_.end() && it->second.channel_ == channel;
    }
}