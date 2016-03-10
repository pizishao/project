#include "Timer.h"

namespace MuduoPlus
{
    std::atomic_int64_t Timer::s_numCreated_ = 0;

    void Timer::restart(Timestamp now)
    {
        if (repeat_)
        {
            expiration_ = addSecondTime(now, interval_);
        }
        else
        {
            expiration_ = Timestamp::invalid();
        }
    }
}