#pragma once

#include "logpp/date/date.h"
#include "logpp/date/tz.h"

#include <chrono>

namespace logpp
{
    using Clock     = std::chrono::system_clock;
    using TimePoint = typename Clock::time_point;

    // A Clock that represents system wall clock and measures time in UTC
    class SystemClock : public std::chrono::system_clock
    {
    public:
        static std::tm* toTm(const time_point& tp)
        {
            auto time = to_time_t(tp);
            return std::gmtime(&time);
        }
    };

    // A Clock that represents sytem wall clock and adjusts time to local time zone
    class LocalClock
    {
    public:
        using Clock = SystemClock;

        using duration   = Clock::duration;
        using time_point = date::local_time<duration>;

        static std::tm* toTm(const time_point& tp)
        {
            auto time = to_time_t(tp);
            return std::gmtime(&time);
        }

        static std::tm* toTm(const TimePoint& tp)
        {
            auto time = SystemClock::to_time_t(tp);
            return std::localtime(&time);
        }

        static time_t to_time_t(const time_point& tp)
        {
            return std::time_t(std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count());
        }
    };
}