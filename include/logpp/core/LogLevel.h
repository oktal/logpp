#pragma once

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

    constexpr inline const char* levelString(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::Trace:
                return "trace";
            case LogLevel::Debug:
                return "debug";
            case LogLevel::Info:
                return "info";
            case LogLevel::Warning:
                return "warn";
            case LogLevel::Error:
                return "error";
        }

        return "none";
    }

}