#pragma once

#include "logpp/format/flag/Formatter.h"
#include "logpp/utils/file.h"

namespace logpp
{
    class SourceFileFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto location = buffer.location();
            if (!location)
                return;

            auto file = file_utils::fileName(location->file);
            out.append(file);
        }
    };

    class SourceLineFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto location = buffer.location();
            if (!location)
                return;

            fmt::format_to(out, "{}", location->line);
        }
    };
}
