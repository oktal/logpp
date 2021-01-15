#include "logpp/format/PatternFormatter.h"

#include "logpp/format/flag/DateFormatter.h"
#include "logpp/format/flag/FieldsFormatter.h"
#include "logpp/format/flag/LevelFormatter.h"
#include "logpp/format/flag/LiteralFormatter.h"
#include "logpp/format/flag/NameFormatter.h"
#include "logpp/format/flag/TextFormatter.h"
#include "logpp/format/flag/TimeFormatter.h"

namespace logpp
{
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
        m_pattern = std::move(pattern);
        m_formatters = parsePattern(m_pattern);
    }

    void PatternFormatter::doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const
    {
        for (const auto& formatter: m_formatters)
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
                        std::tie(it, formatter) = parseFlag(it);
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
            case 'Y':
            {
                ++it;
                formatter = std::make_shared<YearFormatter>();
                break;
            }
            // Writes month as a decimal number (range [01,12]) 
            case 'm':
            {
                ++it;
                formatter = std::make_shared<MonthDecimalFormatter>();
                break;
            }
            // writes day of the month as a decimal number (range [01,31])
            case 'd':
            {
                ++it;
                formatter = std::make_shared<DayDecimalFormatter>();
                break;
            }

            // -----------------------------
            // Time
            // -----------------------------

            // Writes hour as a decimal number, 24 hour clock (range [00-23])
            case 'H':
            {
                ++it;
                formatter = std::make_shared<HoursFormatter>();
                break;
            }
            // Writes minute as a decimal number (range [00,59])
            case 'M':
            {
                ++it;
                formatter = std::make_shared<MinutesFormatter>();
                break;
            }
            // Writes second as a decimal number (range [00,60])
            case 'S':
            {
                ++it;
                formatter = std::make_shared<SecondsFormatter>();
                break;
            }
            // Writes milliseconds as a decimal number (range [000, 999])
            case 'i':
            {
                ++it;
                formatter = std::make_shared<MillisecondsFormatter>();
                break;
            }
            // Writes microseconds as a decimal number (range [000, 999])
            case 'u':
            {
                ++it;
                formatter = std::make_shared<MicrosecondsFormatter>();
                break;
            }

            // Text
            case 'v':
            {
                ++it;
                formatter = std::make_shared<TextFormatter>();
                break;
            }
            // Level
            case 'l':
            {
                ++it;
                formatter = std::make_shared<LevelFormatter>();
                break;
            }
            // Logger name
            case 'n':
            {
                ++it;
                formatter = std::make_shared<NameFormatter>();
                break;
            }
            // Log event fields
            case 'f':
            {
                ++it;
                std::string prefix;
                if (*it == '[')
                {
                    ++it;
                    while (*it != ']')
                    {
                        prefix.push_back(*it++);
                    }
                    ++it;
                }
                formatter = std::make_shared<FieldsFormatter>(std::move(prefix));
                break;
            }
        }


        return std::make_pair(it, std::move(formatter));
    }
}