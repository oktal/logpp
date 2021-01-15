#pragma once

#include "logpp/format/Formatter.h"

#include <fmt/core.h>

namespace logpp
{
    class LogFmtFormatter : public Formatter
    {
    private:
        void doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& dest) const override;
    };
}