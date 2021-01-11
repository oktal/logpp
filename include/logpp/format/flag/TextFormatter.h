#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class TextFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            buffer.formatText(out);
        }
    };
}