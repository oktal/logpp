#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/core/LogLevel.h"
#include "logpp/core/Offset.h"

namespace logpp::sink
{
    class Sink
    {
    public:
        virtual void format(LogLevel level, EventLogBuffer buffer, StringOffset text) = 0;
    };
}