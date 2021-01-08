#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class YearFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:04}", time->tm_year + 1900);
        }
    };

    class MonthDecimalFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_mon + 1);
        }
    };

    class DayDecimalFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_mday);
        }
    };
}