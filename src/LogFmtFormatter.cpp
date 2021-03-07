#include "logpp/format/LogFmtFormatter.h"

#include "logpp/format/flag/DateFormatter.h"
#include "logpp/format/flag/FieldsFormatter.h"
#include "logpp/format/flag/LevelFormatter.h"
#include "logpp/format/flag/LiteralFormatter.h"
#include "logpp/format/flag/NameFormatter.h"
#include "logpp/format/flag/TextFormatter.h"
#include "logpp/format/flag/ThreadFormatter.h"
#include "logpp/format/flag/TimeFormatter.h"

#include "logpp/core/Clock.h"
#include "logpp/core/LogFieldVisitor.h"

#include "logpp/utils/date.h"
#include "logpp/utils/thread.h"

#include <chrono>
#include <fmt/format.h>

namespace logpp
{
    class LogFmtKeyValueFormatter : public FlagFormatter
    {
    public:
        LogFmtKeyValueFormatter(std::string key, std::shared_ptr<FlagFormatter> valueFormatter)
            : m_key(std::move(key))
            , m_valueFormatter(std::move(valueFormatter))
        { }

        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            out.append(m_key);
            out.push_back('=');

            fmt::memory_buffer formatBuf;
            m_valueFormatter->format(name, level, buffer, formatBuf);

            auto str               = std::string_view(formatBuf.data(), formatBuf.size());
            const auto shouldQuote = str.find(' ') != std::string_view::npos;

            if (shouldQuote)
                out.push_back('"');

            out.append(str);

            if (shouldQuote)
                out.push_back('"');
        }

    private:
        std::string m_key;
        std::shared_ptr<FlagFormatter> m_valueFormatter;
    };

    class MultiFlagFormatter : public FlagFormatter
    {
    public:
        MultiFlagFormatter() = default;

        explicit MultiFlagFormatter(std::string separator)
            : m_separator(std::move(separator))
        { }

        MultiFlagFormatter(std::vector<std::shared_ptr<FlagFormatter>> flagFormatters, std::string separator = "")
            : m_flagFormatters(std::move(flagFormatters))
            , m_separator(std::move(separator))
        { }

        template <typename Formatter, typename... Args>
        void add(Args&&... args)
        {
            add(std::make_shared<Formatter>(std::forward<Args>(args)...));
        }

        void add(std::shared_ptr<FlagFormatter> formatter)
        {
            auto proxy = std::make_shared<FlagFormatterProxy>(std::move(formatter));

            const auto shouldPrefix = !m_separator.empty() && m_flagFormatters.size() > 0;
            proxy->setPreFormat([=](std::string_view, LogLevel, const EventLogBuffer&, fmt::memory_buffer& out) {
                if (shouldPrefix)
                    out.append(m_separator.data(), m_separator.data() + m_separator.size());
            });

            m_flagFormatters.push_back(std::move(proxy));
        }

        void add(std::shared_ptr<FieldsFormatter> formatter)
        {
            m_flagFormatters.push_back(std::move(formatter));
        }

        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            for (const auto& flagFormatter : m_flagFormatters)
            {
                flagFormatter->format(name, level, buffer, out);
            }
        }

    private:
        class FlagFormatterProxy : public FlagFormatter
        {
        public:
            using OnFormat = std::function<void(std::string_view, LogLevel, const EventLogBuffer&, fmt::memory_buffer&)>;

            explicit FlagFormatterProxy(std::shared_ptr<FlagFormatter> formatter)
                : m_formatter(std::move(formatter))
            { }

            void setPreFormat(OnFormat onFormat)
            {
                m_onPreFormat = std::move(onFormat);
            }

            void setPostFormat(OnFormat onFormat)
            {
                m_onPreFormat = std::move(onFormat);
            }

            void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
            {
                if (m_onPreFormat)
                    m_onPreFormat(name, level, buffer, out);

                m_formatter->format(name, level, buffer, out);

                if (m_onPostFormat)
                    m_onPostFormat(name, level, buffer, out);
            }

        private:
            std::shared_ptr<FlagFormatter> m_formatter;

            OnFormat m_onPreFormat;
            OnFormat m_onPostFormat;
        };

        std::vector<std::shared_ptr<FlagFormatter>> m_flagFormatters;
        std::string m_separator;
    };

    struct ParseContext
    {
        static constexpr int Eof = -1;

        struct Capture
        {
            explicit Capture(ParseContext& context)
                : context(context)
                , it(context.it)
            { }

            bool empty() const
            {
                return it == context.it;
            }

            size_t size() const
            {
                return std::distance(it, context.it);
            }

            std::string_view value() const
            {
                return std::string_view(&*it, size());
            }

            std::string toString() const
            {
                return std::string(value());
            }

            void reset()
            {
                it = context.it;
            }

            ParseContext& context;
            std::string_view::const_iterator it;
        };

        explicit ParseContext(std::string_view data)
            : data(data)
            , it(std::begin(data))
        { }

        bool eof() const
        {
            return it == std::end(data);
        }

        bool advance(size_t count)
        {
            const auto remaining = static_cast<size_t>(std::distance(it, std::end(data)));
            if (count > remaining)
                return false;

            std::advance(it, count);
            return true;
        }

        char operator*() const
        {
            return current();
        }

        ParseContext& operator++()
        {
            advance(1);
            return *this;
        }

        char current() const
        {
            return *it;
        }

        int next() const
        {
            if (it != std::end(data))
                return *(it + 1);

            return Eof;
        }

        size_t index() const
        {
            return static_cast<size_t>(std::distance(std::begin(data), it));
        }

        size_t remaining() const
        {
            return static_cast<size_t>(std::distance(it, std::end(data)));
        }

        Capture capture()
        {
            return Capture(*this);
        }

        std::string_view data;
        std::string_view::const_iterator it;
    };

    template <typename... Args>
    void throwParseError(ParseContext& context, const char* formatStr, Args&&... args)
    {
        auto what = fmt::format(formatStr, std::forward<Args>(args)...);
        throw LogFmtPatternError(std::move(what), context.index());
    }

    std::string_view matchUntil(ParseContext& context, char c)
    {
        auto startIt = context.it;
        auto& endIt  = context.it;
        while (endIt != std::end(context.data) && *endIt != c)
        {
            ++endIt;
        }

        return std::string_view(&*startIt, std::distance(startIt, endIt));
    }

    bool matchChar(ParseContext& context, char c)
    {
        if (*context.it == c)
        {
            ++context.it;
            return true;
        }

        return false;
    }

    bool matchString(ParseContext& context, std::string_view str)
    {
        if (!context.data.compare(context.index(), str.size(), str))
        {
            context.advance(str.size());
            return true;
        }

        return false;
    }

    std::shared_ptr<FlagFormatter> matchPattern(std::string_view pattern);

    std::shared_ptr<FlagFormatter> matchFlag(ParseContext& context)
    {
        auto formatter = std::make_shared<MultiFlagFormatter>();

        auto capture = context.capture();
        while (!context.eof() && *context != ' ')
        {
            if (*context == '%')
            {
                // We ignore the '%f' flag at this level and match it in `matchPattern` instead.
                // Here is why. Let's suppose that the pattern we are trying to parse is `msg=%v%f`
                // We first match "msg" as the key, and then start to match the flags.
                // We first match %v, then %f, meaning that we will return a `MultiFlagFormatter` with
                // a `TextFormatter` and `FieldsFormatter`.
                //
                // If we attempt to format a log message with spaces, we will thus end up with an invalid quoting:
                // msg="My message with spaces my_field=1" since we will attempt to quote the whole formatted
                // buffer. The expected result should be msg="My message with spaces" my_field=1
                // To avoid that, we "skip" the %f flag if we encounter it and match it on its own.
                if (context.next() == 'f')
                    break;

                if (!capture.empty())
                    formatter->add<LiteralFormatter>(capture.toString());

                if (!context.advance(1))
                    throwParseError(context, "expected flag, got EOF");

                auto flag = *context;
                switch (flag)
                {
                // -----------------------------
                // Date
                // -----------------------------

                // Writes year as a decimal number, e.g. 2017
                case 'Y':
                    formatter->add<YearFormatter>();
                    break;
                // Writes month as a decimal number (range [01,12])
                case 'm': {
                    formatter->add<MonthDecimalFormatter>();
                    break;
                // Writes day of the month as a decimal number (range [01,31])
                case 'd': {
                    formatter->add<DayDecimalFormatter>();
                    break;
                }

                // -----------------------------
                // Time
                // -----------------------------
                // Writes hour as a decimal number, 24 hour clock (range [00-23])
                case 'H': {
                    formatter->add<HoursFormatter>();
                    break;
                }
                // Writes minute as a decimal number (range [00,59])
                case 'M': {
                    formatter->add<MinutesFormatter>();
                    break;
                }
                // Writes second as a decimal number (range [00,60])
                case 'S': {
                    formatter->add<SecondsFormatter>();
                    break;
                }
                // Writes millisecond as a decimal number (range [000, 999])
                case 'i': {
                    formatter->add<MillisecondsFormatter>();
                    break;
                }
                // Writes microsecond as a decimal number (range [000, 999])
                case 'u': {
                    formatter->add<MicrosecondsFormatter>();
                    break;
                }

                // Thread id
                case 't': {
                    formatter->add<ThreadFormatter>();
                    break;
                }

                // Text
                case 'v': {
                    formatter->add<TextFormatter>();
                    break;
                }
                // Level
                case 'l': {
                    formatter->add<LevelFormatter>();
                    break;
                }
                // Logger name
                case 'n': {
                    formatter->add<NameFormatter>();
                    break;
                }
                case '+': {
                    formatter->add(matchPattern(LogFmtFormatter::DefaultPattern));
                    break;
                }
                }

                default:
                    throwParseError(context, "unknown flag '{}'", flag);
                }

                ++context;
                capture.reset();
            }
            else
            {
                ++context;
            }
        }

        if (!capture.empty())
            formatter->add<LiteralFormatter>(capture.toString());

        return formatter;
    }

    std::shared_ptr<FlagFormatter> matchPattern(std::string_view pattern)
    {
        ParseContext context(pattern);
        if (matchString(context, "%+"))
            return matchPattern(LogFmtFormatter::DefaultPattern);

        auto formatter = std::make_shared<MultiFlagFormatter>(" ");
        while (!context.eof())
        {
            if (matchString(context, "%f"))
            {
                formatter->add<FieldsFormatter>(" ");
            }
            else
            {
                auto key = matchUntil(context, '=');
                if (!context.advance(1))
                    throwParseError(context, "expected value, got EOF");

                if (context.eof())
                    throwParseError(context, "expected value, got EOF");

                auto flag = matchFlag(context);
                formatter->add<LogFmtKeyValueFormatter>(std::string(key), std::move(flag));
            }

            while (!context.eof() && *context == ' ')
                ++context;
        }

        return formatter;
    }

    LogFmtFormatter::LogFmtFormatter()
    {
        setPattern("%+");
    }

    LogFmtFormatter::LogFmtFormatter(std::string pattern)
    {
        setPattern(std::move(pattern));
    }

    void LogFmtFormatter::setPattern(std::string pattern)
    {
        m_formatter = matchPattern(pattern);
        m_pattern   = std::move(pattern);
    }

    void LogFmtFormatter::doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& dest) const
    {
        m_formatter->format(name, level, buffer, dest);
    }
}
