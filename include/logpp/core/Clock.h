#pragma once

#include "logpp/date/date.h"
#include "logpp/date/tz.h"

#include <chrono>

namespace logpp
{
    using Clock = std::chrono::system_clock;
    using TimePoint = typename Clock::time_point;

    // A Clock that represents system wall clock and measures time in UTC
    using SystemClock = std::chrono::system_clock;

    // A Clock that represents sytem wall clock and adjusts time to local time zone
    class LocalClock
    {
    public:
        using Clock = SystemClock;

        using duration = Clock::duration;
        using time_point = date::local_time<duration>;

        static time_point now()
        {
            auto utcNow = Clock::now();
            auto now = date::make_zoned(date::current_zone(), utcNow);

            return now.get_local_time();
        }

        static time_t to_time_t(const time_point& tp)
        {
            return std::time_t(std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count());
        }
    };
}