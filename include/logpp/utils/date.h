#pragma once

#include "logpp/core/Clock.h"
#include "logpp/core/date.h"

namespace logpp
{
    namespace date_utils
    {
        inline auto time(TimePoint tp)
        {
            auto dp = date::floor<date::days>(tp);
            return date::make_time(tp - dp);
        }

        inline date::year_month_day date(TimePoint tp)
        {
            return { date::floor<date::days>(tp) };
        }

        inline int year(TimePoint tp)
        {
            return static_cast<int>(date(tp).year());
        }

        inline unsigned month(TimePoint tp)
        {
            return static_cast<unsigned>(date(tp).month());
        }

        inline unsigned day(TimePoint tp)
        {
            return static_cast<unsigned>(date(tp).day());
        }

        inline auto hours(TimePoint tp)
        {
            return time(tp).hours().count();
        }

        inline auto minutes(TimePoint tp)
        {
            return time(tp).minutes().count();
        }

        inline auto seconds(TimePoint tp)
        {
            return time(tp).seconds().count();
        }
    }
}