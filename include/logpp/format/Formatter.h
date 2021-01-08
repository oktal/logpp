#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/core/LogLevel.h"

#include <fmt/format.h>

namespace logpp
{
    class Formatter
    {
    public:
        virtual ~Formatter() = default;

        virtual void format(std::string_view name, LogLevel debug, const EventLogBuffer& buffer, fmt::memory_buffer& dest) const = 0;
    };
}