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

        template<typename Formatter>
        std::shared_ptr<Formatter> getFlagFormatter()
        {
            static_assert(std::is_base_of_v<FlagFormatter, Formatter>, "Formatter should be a FlagFormatter");

            for (const auto& formatter: m_formatters)
            {
                auto ptr = std::dynamic_pointer_cast<Formatter>(formatter);
                if (ptr)
                    return ptr;
            }

            return nullptr;
        }

        void setPattern(std::string pattern);
        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override;

    private:
        std::string m_pattern;
        std::vector<std::shared_ptr<FlagFormatter>> m_formatters;

        static std::vector<std::shared_ptr<FlagFormatter>> parsePattern(const std::string& pattern);
        static std::pair<std::string::const_iterator, std::shared_ptr<FlagFormatter>> parseFlag(std::string::const_iterator it);
    };
}