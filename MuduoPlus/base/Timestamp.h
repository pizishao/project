#pragma once

#include <inttypes.h>
#include <stdint.h>
#include <algorithm>
#include <string>

#include "Copyable.h"

namespace MuduoPlus
{
    class Timestamp : public Copyable
    {
    public:
        Timestamp()
            : microSecondsSinceEpoch_(0)
        {
        }

        explicit Timestamp(int64_t microSecondsSinceEpochArg)
            : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
        {
        }

        void swap(Timestamp& that)
        {
            std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
        }

        std::string toString() const;
        std::string toFormattedString(bool showMicroseconds = true) const;

        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
        time_t secondsSinceEpoch() const
        {
            return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecPerSec);
        }

        inline Timestamp addSeconds(double seconds)
        {
            int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecPerSec);
            microSecondsSinceEpoch_ += delta;

            return *this;
        }

        inline Timestamp addMillionSeconds(double millionSeconds)
        {
            microSecondsSinceEpoch_ += static_cast<int64_t>(millionSeconds * kMicroSecPerMilliSec);

            return *this;
        }

        inline Timestamp addMicroSeconds(double microSeconds)
        {
            microSecondsSinceEpoch_ += static_cast<int64_t>(microSeconds);

            return *this;
        }

        static Timestamp now();

        double secondFromNow()
        {
            Timestamp nowStamp = Timestamp::now();
            int64_t diff = nowStamp.microSecondsSinceEpoch() - microSecondsSinceEpoch();
            return static_cast<double>(diff) / Timestamp::kMicroSecPerSec;
        }

        double milliSecondFromNow()
        {
            Timestamp nowStamp = Timestamp::now();
            int64_t diff = microSecondsSinceEpoch() - nowStamp.microSecondsSinceEpoch();
            return static_cast<double>(diff) / Timestamp::kMicroSecPerMilliSec;
        }

        int64_t microSecondFromNow()
        {
            Timestamp nowStamp = Timestamp::now();
            int64_t diff = nowStamp.microSecondsSinceEpoch() - microSecondsSinceEpoch();
            return diff;
        }

        static Timestamp invalid()
        {
            return Timestamp();
        }

        static Timestamp fromUnixTime(time_t t)
        {
            return fromUnixTime(t, 0);
        }

        static Timestamp fromUnixTime(time_t t, int microseconds)
        {
            return Timestamp(static_cast<int64_t>(t)* kMicroSecPerSec + microseconds);
        }

        static const int kMicroSecPerSec        = 1000 * 1000;
        static const int kMicroSecPerMilliSec   = 1000;

    private:
        int64_t microSecondsSinceEpoch_;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }

    inline double secondDifference(Timestamp high, Timestamp low)
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecPerSec;
    }

    inline double millisecondDifference(Timestamp high, Timestamp low)
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecPerMilliSec;
    }

    inline int64_t microSecondDifference(Timestamp high, Timestamp low)
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return diff;
    }    
}

