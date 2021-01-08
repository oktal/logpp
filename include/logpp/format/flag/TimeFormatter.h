#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class HoursFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_hour);
        }
    };

    class MinutesFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_min);
        }
    };

    class SecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_sec);
        }
    };
}