#pragma once

#include "logpp/core/Clock.h"
#include "logpp/sinks/file/File.h"
#include "logpp/utils/date.h"

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

    enum class RollingKind
    {
        Precise,
        Round
    };

    inline std::string_view toString(RollingInterval interval)
    {
        switch (interval)
        {
            case RollingInterval::Minute:
                return "Minute";
            case RollingInterval::Hour:
                return "Hour";
            case RollingInterval::Day:
                return "Day";
            case RollingInterval::Month:
                return "Month";
            case RollingInterval::Year:
                return "Year";
            case RollingInterval::Infinite:
                return "Infinite";
        }

        return "";
    }

    inline std::string_view toString(RollingKind kind)
    {
        switch (kind)
        {
            case RollingKind::Precise:
                return "Precise";
            case RollingKind::Round:
                return "Round";
        }

        return "";
    }

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
        explicit DateRollingStrategy(RollingInterval interval, RollingKind kind)
            : interval(interval)
            , kind(kind)
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
                nextRollTime = computeNextRollTime(tp);
                return true;
            }

            return false;
        }

    private: 
        RollingInterval interval;
        RollingKind kind;
        std::optional<TimePoint> nextRollTime;

        template<typename Rep, typename Period>
        TimePoint computeNextRollTime(TimePoint tp, const std::chrono::duration<Rep, Period>& incr) const
        {
            using Duration = std::chrono::duration<Rep, Period>;

            return kind == RollingKind::Precise ?
                tp + incr :
                std::chrono::ceil<Duration>(tp);
        }

        TimePoint computeNextRollTime(TimePoint tp) const
        {
            switch (interval)
            {
                case RollingInterval::Minute:
                    return computeNextRollTime(tp, std::chrono::minutes(1));
                case RollingInterval::Hour:
                    return computeNextRollTime(tp, std::chrono::hours(1));
                case RollingInterval::Day:
                    return computeNextRollTime(tp, date::days(1));
                case RollingInterval::Month:
                    return computeNextRollTime(tp, date::months(1));
                case RollingInterval::Year:
                    return computeNextRollTime(tp, date::years(1));
                case RollingInterval::Infinite:
                    throw std::runtime_error("Can not compute next rolling time with infinite interval");
            }

            throw std::runtime_error("Unknown interval");
        }

    };
}