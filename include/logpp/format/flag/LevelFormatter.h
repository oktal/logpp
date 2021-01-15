#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class LevelFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view, LogLevel level, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            auto str = levelString(level);
            out.append(str.data(), str.data() + str.size());
        }
    };
}