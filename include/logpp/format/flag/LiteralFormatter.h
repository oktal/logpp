#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class LiteralFormatter : public FlagFormatter
    {
    public:
        explicit LiteralFormatter(std::string str)
            : m_str(str)
        {}

        virtual void format(std::string_view, LogLevel, const std::tm*, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            out.append(m_str.data(), m_str.data() + m_str.size());
        }

    private:
        std::string m_str;
    };
}

