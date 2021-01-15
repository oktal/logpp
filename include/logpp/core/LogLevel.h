#pragma once

#include "logpp/utils/string.h"

#include <string_view>
#include <optional>

namespace logpp
{
    enum class LogLevel
    {
        Trace,
        Debug,
        Info,
        Warning,
        Error
    };

    constexpr inline std::string_view levelString(LogLevel level)
    {
        using namespace std::string_view_literals;

        switch (level)
        {
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
        if (string_utils::iequals(level, "trace"))
            return LogLevel::Trace;
        else if (string_utils::iequals(level, "debug"))
            return LogLevel::Debug;
        else if (string_utils::iequals(level, "info"))
            return LogLevel::Info;
        else if (string_utils::iequals(level, "warn"))
            return LogLevel::Warning;
        else if (string_utils::iequals(level, "error"))
            return LogLevel::Error;

        return std::nullopt;
    }
}