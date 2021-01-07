#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/core/LogLevel.h"
#include "logpp/core/Offset.h"

namespace logpp::sink
{
    class Sink
    {
    public:
        virtual void format(std::string_view name, LogLevel level, EventLogBuffer buffer) = 0;
    };
}