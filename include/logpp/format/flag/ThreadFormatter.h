#pragma once

#include "logpp/format/flag/Formatter.h"
#include "logpp/utils/thread.h"

namespace logpp
{
    class ThreadFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{}", thread_utils::toInteger(buffer.threadId()));
        }
    };
}
