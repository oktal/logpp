#pragma once

#include "logpp/format/Formatter.h"
#include "logpp/format/flag/Formatter.h"
#include "logpp/format/flag/TimeZone.h"

#include <type_traits>

namespace logpp
{
    class PatternFormatter : public Formatter
    {
    public:
        PatternFormatter();
        explicit PatternFormatter(std::string pattern);

        template <typename Formatter>
        std::shared_ptr<Formatter> getFlagFormatter()
        {
            static_assert(std::is_base_of_v<FlagFormatter, Formatter>, "Formatter should be a FlagFormatter");

            for (const auto& formatter : m_formatters)
            {
                auto ptr = std::dynamic_pointer_cast<Formatter>(formatter);
                if (ptr)
                    return ptr;
            }

            return nullptr;
        }

        void setPattern(std::string pattern);

        template <typename F>
        static bool registerFlag(char flag)
        {
            static_assert(
                std::is_base_of_v<FlagFormatter, F>,
                "Formatter must inherit from FlagFormatter");

            static_assert(
                std::is_constructible_v<F, std::string>,
                "Formatter must be constructible from a string type (either std::string or std::string_view) ");

            auto factory = [](std::string param) {
                return std::make_shared<F>(std::move(param));
            };

            return m_customFormatters.insert(std::make_pair(flag, std::move(factory))).second;
        }

    private:
        struct FlagParseResult
        {
            std::string::const_iterator it;

            bool result;
            std::shared_ptr<FlagFormatter> formatter;

            static FlagParseResult ok(std::string::const_iterator it, std::shared_ptr<FlagFormatter> formatter)
            {
                return { it, true, std::move(formatter) };
            }

            static FlagParseResult error(std::string::const_iterator it)
            {
                return  { it, false, nullptr };
            }

            bool isOk() const
            {
                return result;
            }
        };

        using FlagFormatterFactory = std::function<std::shared_ptr<FlagFormatter>(std::string)>;
        static std::unordered_map<char, FlagFormatterFactory> m_customFormatters;

        std::string m_pattern;
        std::vector<std::shared_ptr<FlagFormatter>> m_formatters;

        tz::ZoneId m_zoneId;

        void doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override;

        std::vector<std::shared_ptr<FlagFormatter>> parsePattern(const std::string& pattern);
        FlagParseResult parseFlag(std::string::const_iterator it);
        std::pair<std::string::const_iterator, std::string>
        parseFlagParameter(std::string::const_iterator it);
    };
}
