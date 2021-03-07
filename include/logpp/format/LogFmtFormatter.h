#pragma once

#include "logpp/format/Formatter.h"
#include "logpp/format/flag/Formatter.h"

#include <fmt/core.h>
#include <string_view>

namespace logpp
{
    class LogFmtPatternError : public std::runtime_error
    {
    public:
        explicit LogFmtPatternError(std::string_view what, size_t column)
            : runtime_error(formatError(what, column))
            , m_column(column)
        {}

        size_t column() const
        {
            return m_column;
        }

    private:
        static std::string formatError(std::string_view what, size_t column)
        {
            return fmt::format("{}: column {}", what, column);
        }

        size_t m_column;
    };

    class LogFmtFormatter : public Formatter
    {
    public:
        static constexpr std::string_view DefaultPattern
            = "ts=%Y-%m-%dT%H:%M:%S lvl=%l logger=%n msg=%v%f";

        LogFmtFormatter();
        explicit LogFmtFormatter(std::string pattern);

        void setPattern(std::string pattern);

    private:
        std::string m_pattern;
        std::shared_ptr<FlagFormatter> m_formatter;

        void doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& dest) const override;
    };
}
