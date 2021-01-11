#pragma once

#include "logpp/format/flag/Formatter.h"
#include "logpp/utils/date.h"

namespace logpp
{
    class YearFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:04}", date_utils::year(buffer.time()));
        }
    };

    class MonthDecimalFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", date_utils::month(buffer.time()));
        }
    };

    class DayDecimalFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", date_utils::day(buffer.time()));
        }
    };
}