#pragma once

#include "logpp/core/Clock.h"
#include "logpp/date/date.h"

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

        template<typename Clock, typename Dur>
        constexpr auto ceil_month(const std::chrono::time_point<Clock, Dur>& tp)
        {
            auto r = std::chrono::floor<date::days>(tp);
            date::year_month_day ymd { r };

            auto month = ymd.month();
            if (month == date::December)
                month = date::January;
            else
                ++month;

            date::year_month_day firstOfNextMonth { ymd.year(), month, date::day{1} };
            return static_cast<date::sys_days>(firstOfNextMonth);
        }

        template<typename Clock, typename Dur>
        constexpr auto ceil_year(const std::chrono::time_point<Clock, Dur>& tp)
        {
            auto r = std::chrono::floor<date::days>(tp);
            date::year_month_day ymd { r };

            auto year = ymd.year();
            ++year;

            date::year_month_day firstOfNextYear { year, date::January, date::day{1} };
            return static_cast<date::sys_days>(firstOfNextYear);
        }
    }
}