#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class HoursFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_hour);
        }
    };

    class MinutesFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_min);
        }
    };

    class SecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm* time, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:02}", time->tm_sec);
        }
    };

    class MillisecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm*, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = buffer.time();
            auto epoch = time.time_since_epoch();

            epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
            fmt::format_to(out, "{:03}", ms.count());
        }
    };

    class MicrosecondsFormatter : public FlagFormatter
    {
        void format(std::string_view, LogLevel, const std::tm*, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            auto time = buffer.time();
            auto epoch = time.time_since_epoch();

            epoch -= std::chrono::duration_cast<std::chrono::seconds>(epoch);
            epoch -= std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

            auto us = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
            fmt::format_to(out, "{:03}", us.count());
        }
    };
}