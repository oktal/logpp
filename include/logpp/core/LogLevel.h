#pragma once

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
                return "Trace"sv;
            case LogLevel::Debug:
                return "Debug"sv;
            case LogLevel::Info:
                return "Info"sv;
            case LogLevel::Warning:
                return "Warn"sv;
            case LogLevel::Error:
                return "Error"sv;
        }

        return "none";
    }

    inline std::optional<LogLevel> parseLevel(std::string level)
    {
        for (auto& c: level)
            c = std::tolower(c);

        if (level == "trace")
            return LogLevel::Trace;
        else if (level == "debug")
            return LogLevel::Debug;
        else if (level == "info")
            return LogLevel::Info;
        else if (level == "warn")
            return LogLevel::Warning;
        else if (level == "error")
            return LogLevel::Error;

        return std::nullopt;
    }
}