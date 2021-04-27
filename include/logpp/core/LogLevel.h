#pragma once

#include "logpp/utils/string.h"

#include <optional>
#include <string_view>

namespace logpp
{
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Off
    };

    constexpr inline std::string_view levelString(LogLevel level)
    {
        using namespace std::string_view_literals;

        switch (level)
        {
        case LogLevel::Off:
            return "off"sv;
        case LogLevel::Trace:
            return "trace"sv;
        case LogLevel::Debug:
            return "debug"sv;
        case LogLevel::Info:
            return "info"sv;
        case LogLevel::Warning:
            return "warn"sv;
        case LogLevel::Error:
            return "error"sv;
        }

        return "none";
    }

    inline std::optional<LogLevel> parseLevel(std::string_view level)
    {
        if (string_utils::iequals(level, "off"))
            return LogLevel::Off;
        if (string_utils::iequals(level, "trace"))
            return LogLevel::Trace;
        if (string_utils::iequals(level, "debug"))
            return LogLevel::Debug;
        if (string_utils::iequals(level, "info"))
            return LogLevel::Info;
        if (string_utils::iequals(level, "warn"))
            return LogLevel::Warning;
        if (string_utils::iequals(level, "error"))
            return LogLevel::Error;

        return std::nullopt;
    }
}
