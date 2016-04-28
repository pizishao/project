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
        ChannelHolderMap::const_iterator it = channelHolders_.find(channel->fd());
        return it != channelHolders_.end() && it->second.channel_ == channel;
    }
}