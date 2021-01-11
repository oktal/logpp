#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class NameFormatter : public FlagFormatter
    {
    public:
        void format(std::string_view name, LogLevel, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            out.append(name.data(), name.data() + name.size());
        }
    };
}