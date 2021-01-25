#pragma once

#include "logpp/core/Clock.h"
#include "logpp/sinks/file/File.h"

namespace logpp::sink
{
    enum class RollingInterval
    {
        Minute,
        Hour,
        Day,
        Month,
        Year,
        Infinite
    };

    class RollingStrategy
    {
    public:
        virtual ~RollingStrategy() = default;

        virtual bool apply(TimePoint tp, const File* file) = 0;
    };

    class SizeRollingStrategy : public RollingStrategy
    {
    public:
        explicit SizeRollingStrategy(size_t bytesThreshold)
            : bytesThreshold(bytesThreshold)
        {}

        bool apply(TimePoint, const File* file)
        {
            return file->size() >= bytesThreshold;
        }

        size_t bytesThreshold;
    };

    class DateRollingStrategy : public RollingStrategy
    {
    public:
        explicit DateRollingStrategy(RollingInterval interval)
            : interval(interval)
        {}

        bool apply(TimePoint tp, const File*)
        {
            if (interval == RollingInterval::Infinite)
                return false;

            if (!nextRollTime)
            {
                nextRollTime = computeNextRollTime(tp);
            }
            else if (tp >= *nextRollTime)
            {
                nextRollTime = computeNextRollTime(*nextRollTime);
                return true;
            }

            return false;
        }

    private: 
        RollingInterval interval;
        std::optional<TimePoint> nextRollTime;

        TimePoint computeNextRollTime(TimePoint time) const
        {
            switch (interval)
            {
                case RollingInterval::Minute:
                    return time + std::chrono::minutes(1);
                case RollingInterval::Hour:
                    return time + std::chrono::hours(1);
            }

            return time;
        }
    };
}