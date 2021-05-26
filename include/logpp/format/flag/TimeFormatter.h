#pragma once

#include "logpp/format/flag/Formatter.h"
#include "logpp/utils/date.h"

namespace logpp
{
    template<typename Tz>
    class HoursFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = Tz::apply(buffer.time());
            fmt::format_to(out, "{:02}", date_utils::hours(time));
        }
    };

    template<typename Tz>
    class MinutesFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = Tz::apply(buffer.time());
            fmt::format_to(out, "{:02}", date_utils::minutes(time));
        }
    };

    template<typename Tz>
    class SecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = Tz::apply(buffer.time());
            fmt::format_to(out, "{:02}", date_utils::seconds(time));
        }
    };

    template<typename Tz>
    class MillisecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time  = Tz::apply(buffer.time());
            auto epoch = time.time_since_epoch();

            epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
            fmt::format_to(out, "{:03}", ms.count());
        }
    };

    template<typename Tz>
    class MicrosecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time  = Tz::apply(buffer.time());
            auto epoch = time.time_since_epoch();

            epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);
            epoch -= std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

            auto us = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
            fmt::format_to(out, "{:03}", us.count());
        }
    };
}
