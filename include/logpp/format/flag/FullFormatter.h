#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class FullFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view name, LogLevel level, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:04}-{:02}-{:02} {:02}:{:02}:{:02} [{}]",
                time->tm_year + 1900,
                time->tm_mon + 1,
                time->tm_mday,
                time->tm_hour,
                time->tm_min,
                time->tm_sec,
                levelString(level)
            );

            if (!name.empty())
                fmt::format_to(out, " {} -", name);

            out.push_back(' ');
            buffer.formatText(out);
        }
    };
}