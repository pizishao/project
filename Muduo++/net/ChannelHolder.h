#pragma once

#include <memory>

namespace MuduoPlus
{
    class Channel;

    struct ChannelHolder
    {
        Channel                     *channel_;
        std::shared_ptr<void>       ower_;
    };
}