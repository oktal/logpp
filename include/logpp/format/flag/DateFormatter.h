#pragma once

#include "logpp/format/flag/Formatter.h"
#include "logpp/utils/date.h"

namespace logpp
{
    template<typename Tz>
    class YearFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = Tz::apply(buffer.time());
            fmt::format_to(out, "{:04}", date_utils::year(time));
        }
    };

    template<typename Tz>
    class MonthDecimalFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = Tz::apply(buffer.time());
            fmt::format_to(out, "{:02}", date_utils::month(time));
        }
    };

    template<typename Tz>
    class DayDecimalFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = Tz::apply(buffer.time());
            fmt::format_to(out, "{:02}", date_utils::day(time));
        }
    };
}
