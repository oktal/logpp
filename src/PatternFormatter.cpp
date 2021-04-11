#include "logpp/format/PatternFormatter.h"

#include "logpp/format/flag/DateFormatter.h"
#include "logpp/format/flag/FieldsFormatter.h"
#include "logpp/format/flag/LevelFormatter.h"
#include "logpp/format/flag/LiteralFormatter.h"
#include "logpp/format/flag/NameFormatter.h"
#include "logpp/format/flag/SourceLocationFormatter.h"
#include "logpp/format/flag/TextFormatter.h"
#include "logpp/format/flag/ThreadFormatter.h"
#include "logpp/format/flag/TimeFormatter.h"

namespace logpp
{
    std::unordered_map<char, PatternFormatter::FlagFormatterFactory>
        PatternFormatter::m_customFormatters;

    PatternFormatter::PatternFormatter()
    {
        setPattern("%+");
    }

    PatternFormatter::PatternFormatter(std::string pattern)
    {
        setPattern(std::move(pattern));
    }

    void PatternFormatter::setPattern(std::string pattern)
    {
        m_pattern    = std::move(pattern);
        m_formatters = parsePattern(m_pattern);
    }

    void PatternFormatter::doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const
    {
        for (const auto& formatter : m_formatters)
        {
            formatter->format(name, level, buffer, out);
        }
    }

    std::vector<std::shared_ptr<FlagFormatter>> PatternFormatter::parsePattern(const std::string& pattern)
    {
        auto it = pattern.begin(), end = pattern.end();

        std::vector<std::shared_ptr<FlagFormatter>> formatters;

        std::string literalStr;

        while (it != end)
        {
            if (*it == '%')
            {
                if (!literalStr.empty())
                    formatters.push_back(std::make_shared<LiteralFormatter>(std::move(literalStr)));

                ++it;
                if (it != end)
                {
                    if (*it == '+')
                    {
                        auto fullFormatters = parsePattern("%Y-%m-%d %H:%M:%S [%l] (%n) %v%f[ - ]");
                        std::copy(std::begin(fullFormatters), std::end(fullFormatters), std::back_inserter(formatters));
                        ++it;
                    }
                    else
                    {
                        std::shared_ptr<FlagFormatter> formatter;
                        auto flag               = *it;
                        std::tie(it, formatter) = parseFlag(it);
                        if (!formatter)
                            throw std::runtime_error(fmt::format("Unknown flag formatter '%{}'", flag));
                        formatters.push_back(std::move(formatter));
                    }
                }
            }
            else
            {
                literalStr.push_back(*it);
                ++it;
            }
        }

        if (!literalStr.empty())
            formatters.push_back(std::make_shared<LiteralFormatter>(std::move(literalStr)));

        return formatters;
    }

    std::pair<std::string::const_iterator, std::shared_ptr<FlagFormatter>>
    PatternFormatter::parseFlag(std::string::const_iterator it)
    {
        std::shared_ptr<FlagFormatter> formatter;

        switch (*it)
        {
        // -----------------------------
        // Date
        // -----------------------------

        // Writes year as a decimal number, e.g. 2017
        case 'Y': {
            ++it;
            formatter = std::make_shared<YearFormatter>();
            break;
        }
        // Writes month as a decimal number (range [01,12])
        case 'm': {
            ++it;
            formatter = std::make_shared<MonthDecimalFormatter>();
            break;
        }
        // Writes day of the month as a decimal number (range [01,31])
        case 'd': {
            ++it;
            formatter = std::make_shared<DayDecimalFormatter>();
            break;
        }

        // -----------------------------
        // Time
        // -----------------------------

        // Writes hour as a decimal number, 24 hour clock (range [00-23])
        case 'H': {
            ++it;
            formatter = std::make_shared<HoursFormatter>();
            break;
        }
        // Writes minute as a decimal number (range [00,59])
        case 'M': {
            ++it;
            formatter = std::make_shared<MinutesFormatter>();
            break;
        }
        // Writes second as a decimal number (range [00,60])
        case 'S': {
            ++it;
            formatter = std::make_shared<SecondsFormatter>();
            break;
        }
        // Writes millisecond as a decimal number (range [000, 999])
        case 'i': {
            ++it;
            formatter = std::make_shared<MillisecondsFormatter>();
            break;
        }
        // Writes microsecond as a decimal number (range [000, 999])
        case 'u': {
            ++it;
            formatter = std::make_shared<MicrosecondsFormatter>();
            break;
        }

        // Thread id
        case 't': {
            ++it;
            formatter = std::make_shared<ThreadFormatter>();
            break;
        }

        // Text
        case 'v': {
            ++it;
            formatter = std::make_shared<TextFormatter>();
            break;
        }
        // Level
        case 'l': {
            ++it;
            formatter = std::make_shared<LevelFormatter>();
            break;
        }
        // Logger name
        case 'n': {
            ++it;
            formatter = std::make_shared<NameFormatter>();
            break;
        }
        // Writes source location file (path)
        case 'p': {
            ++it;
            formatter = std::make_shared<SourceFileFormatter>();
            break;
        }
        // Writes source location line
        case 'o': {
            ++it;
            formatter = std::make_shared<SourceLineFormatter>();
            break;
        }
        // Log event fields
        case 'f': {
            ++it;
            std::string prefix;
            std::tie(it, prefix) = parseFlagParameter(it);
            formatter            = std::make_shared<FieldsFormatter>(std::move(prefix));
            break;
        }
        // Custom
        default: {
            auto customIt = m_customFormatters.find(*it);
            if (customIt != std::end(m_customFormatters))
            {
                ++it;
                std::string param;
                std::tie(it, param) = parseFlagParameter(it);
                auto factory        = customIt->second;
                formatter           = factory(std::move(param));
            }
        }
        }

        return std::make_pair(it, std::move(formatter));
    }

    std::pair<std::string::const_iterator, std::string>
    PatternFormatter::parseFlagParameter(std::string::const_iterator it)
    {
        std::string param;
        if (*it == '[')
        {
            ++it;
            while (*it != ']')
            {
                param.push_back(*it++);
            }
            ++it;
        }
        return std::make_pair(it, std::move(param));
    }
}
