#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/core/LogLevel.h"

#include <ctime>

namespace logpp
{
    class FlagFormatter
    {
    public:
        virtual ~FlagFormatter() = default;

        virtual void format(std::string_view name, LogLevel level, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const = 0;
    };
}