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

}