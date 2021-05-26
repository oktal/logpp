#pragma once

#include "logpp/core/Clock.h"
#include "logpp/utils/date.h"

namespace logpp::tz
{
    enum class ZoneId
    {
        Utc,
        Local
    };

    struct Utc
    {
        static TimePoint apply(TimePoint utcTime)
        {
            return utcTime;
        }
    };

    struct Local
    {
        static TimePoint apply(TimePoint utcTime)
        {
            auto tt = Clock::to_time_t(utcTime);
            std::tm tm;
            date_utils::localtime(&tt, &tm);
            return Clock::from_time_t(date_utils::timegm(&tm));
        }
    };

}
