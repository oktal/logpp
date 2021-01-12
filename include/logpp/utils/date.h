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

        inline auto year(TimePoint tp)
        {
            date::year_month_day ymd { date::floor<date::days>(tp) };
            return static_cast<int>(ymd.year());
        }

        inline auto month(TimePoint tp)
        {
            date::year_month_day ymd { date::floor<date::days>(tp) };
            return static_cast<unsigned>(ymd.month());
        }

        inline auto day(TimePoint tp)
        {
            date::year_month_day ymd { date::floor<date::days>(tp) };
            return static_cast<unsigned>(ymd.day());
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