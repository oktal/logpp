#pragma once

#include "logpp/format/Formatter.h"
#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class PatternFormatter : public Formatter
    {
    public:
        PatternFormatter();
        explicit PatternFormatter(std::string pattern);

        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override;

    private:
        std::string m_pattern;
        std::vector<std::unique_ptr<FlagFormatter>> m_formatters;

        void parsePattern(const std::string& pattern);
        static std::pair<std::string::const_iterator, std::unique_ptr<FlagFormatter>> parseFlag(std::string::const_iterator it);
    };
}