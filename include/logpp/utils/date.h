#pragma once

#include "logpp/core/Clock.h"
#include "logpp/core/config.h"
#include "logpp/date/date.h"

#include <time.h>

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

        inline void gmtime(std::time_t* time, std::tm* out)
        {
#if defined(LOGPP_PLATFORM_LINUX)
            ::gmtime_r(time, out);
#elif defined(LOGPP_PLATFORM_WINDOWS)
            gmtime_s(out, time);
#else
#error "Unknown platform"
#endif
        }

        inline void localtime(std::time_t* time, std::tm* out)
        {
#if defined(LOGPP_PLATFORM_LINUX)
            ::localtime_r(time, out);
#elif defined(LOGPP_PLATFORM_WINDOWS)
            localtime_s(out, time);
#else
#error "Unknown platform"
#endif
        }

        inline time_t timegm(std::tm* tm)
        {
#if defined(LOGPP_PLATFORM_LINUX)
            return ::timegm(tm);
#elif defined(LOGPP_PLATFORM_WINDOWS)
            return _mkgmtime(tm);
#else
#error "Unknown platform"
#endif
        }

        namespace details
        {
            template <typename Duration>
            using period = typename Duration::period;

            template <typename D1, typename D2>
            using is_duration_equal = std::ratio_equal<period<D1>, period<D2>>;

            template <typename D1, typename D2>
            inline constexpr bool is_duration_equal_v
                = is_duration_equal<D1, D2>::value;

            template <typename Clock, typename Dur>
            constexpr std::chrono::time_point<Clock, Dur> floor_month(const std::chrono::time_point<Clock, Dur>& tp)
            {
                auto r = std::chrono::floor<date::days>(tp);
                date::year_month_day ymd { r };

                date::year_month_day firstOfMonth { ymd.year(), ymd.month(), date::day { 1 } };
                return static_cast<date::sys_days>(firstOfMonth);
            }

            template <typename Clock, typename Dur>
            constexpr std::chrono::time_point<Clock, Dur> floor_year(const std::chrono::time_point<Clock, Dur>& tp)
            {
                auto r = std::chrono::floor<date::days>(tp);
                date::year_month_day ymd { r };

                date::year_month_day firstOfYear { ymd.year(), date::January, date::day { 1 } };
                return static_cast<date::sys_days>(firstOfYear);
            }

            template <typename Clock, typename Dur>
            constexpr std::chrono::time_point<Clock, Dur> ceil_month(const std::chrono::time_point<Clock, Dur>& tp)
            {
                auto r = std::chrono::floor<date::days>(tp);
                date::year_month_day ymd { r };

                auto m = ymd.month();
                if (m == date::December)
                    m = date::January;
                else
                    ++m;

                date::year_month_day firstOfNextMonth { ymd.year(), m, date::day { 1 } };
                return static_cast<date::sys_days>(firstOfNextMonth);
            }

            template <typename Clock, typename Dur>
            constexpr std::chrono::time_point<Clock, Dur> ceil_year(const std::chrono::time_point<Clock, Dur>& tp)
            {
                auto r = std::chrono::floor<date::days>(tp);
                date::year_month_day ymd { r };

                auto y = ymd.year();
                ++y;

                date::year_month_day firstOfNextYear { y, date::January, date::day { 1 } };
                return static_cast<date::sys_days>(firstOfNextYear);
            }
        }

        template <class ToDuration, class Clock, class Duration>
        constexpr auto floor(const std::chrono::time_point<Clock, Duration>& tp)
        {
            if constexpr (details::is_duration_equal_v<ToDuration, date::months>)
                return details::floor_month(tp);
            else if constexpr (details::is_duration_equal_v<ToDuration, date::years>)
                return details::floor_year(tp);
            else
                return std::chrono::floor<ToDuration>(tp);
        }

        template <class ToDuration, class Clock, class Duration>
        constexpr auto ceil(const std::chrono::time_point<Clock, Duration>& tp)
        {
            if constexpr (details::is_duration_equal_v<ToDuration, date::months>)
                return details::ceil_month(tp);
            else if constexpr (details::is_duration_equal_v<ToDuration, date::years>)
                return details::ceil_year(tp);
            else
                return std::chrono::ceil<ToDuration>(tp);
        }
    }
}
