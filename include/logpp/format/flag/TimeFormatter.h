#pragma once

#include "logpp/format/flag/Formatter.h"
#include "logpp/utils/date.h"

namespace logpp
{
    class HoursFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", date_utils::hours(buffer.time()));
        }
    };

    class MinutesFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", date_utils::minutes(buffer.time()));
        }
    };

    class SecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", date_utils::seconds(buffer.time()));
        }
    };

    class MillisecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time  = buffer.time();
            auto epoch = time.time_since_epoch();

            epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
            fmt::format_to(out, "{:03}", ms.count());
        }
    };

    class MicrosecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time  = buffer.time();
            auto epoch = time.time_since_epoch();

            epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);
            epoch -= std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

            auto us = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
            fmt::format_to(out, "{:03}", us.count());
        }
    };
}