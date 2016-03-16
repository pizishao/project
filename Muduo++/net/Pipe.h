#pragma once

#include <memory>

namespace MuduoPlus
{
    class Channel;

    struct Pipe
    {
        Channel                     *channel_;
        std::shared_ptr<void>       ower_;
    };
}