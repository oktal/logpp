#pragma once

#include "logpp/core/Offset.h"
#include "logpp/core/Clock.h"

#include <string_view>

namespace logpp
{
    class LogWriter
    {
    public:
        virtual void write(std::string_view key, LogBufferView view, StringOffset text) = 0;

        // Generate implementation for integer offset types
        #define INTEGER_OFFSET(T) \
            virtual void write(std::string_view key, LogBufferView view, Offset<T> offset) = 0;
        INTEGER_OFFSET_TYPES
        #undef INTEGER_OFFSET

        virtual void write(std::string_view key, LogBufferView view, Offset<double> offset) = 0;
    };
}