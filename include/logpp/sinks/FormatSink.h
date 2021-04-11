#pragma once

#include "logpp/format/Formatter.h"
#include "logpp/format/LogFmtFormatter.h"
#include "logpp/format/PatternFormatter.h"

#include "logpp/sinks/Sink.h"

#include <fmt/format.h>

namespace logpp::sink
{

    class FormatSink : public SinkBase
    {
    public:
        explicit FormatSink(std::shared_ptr<Formatter> formatter)
            : m_formatter(std::move(formatter))
        { }

        void setPattern(std::string pattern)
        {
            createFormatter<PatternFormatter>(std::move(pattern));
        }

        void activateOptions(const Options& options) override
        {
            for (auto&& [key, value] : options)
            {
                if (string_utils::iequals(key, "format"))
                {
                    auto formatStr = value.asString();
                    if (!formatStr)
                        raiseConfigurationError("format: expected string");

                    if (string_utils::iequals(*formatStr, "pattern"))
                        createFormatter<PatternFormatter>();
                    else if (string_utils::iequals(*formatStr, "logfmt"))
                        createFormatter<LogFmtFormatter>();
                    else
                        raiseConfigurationError("format: unknown formatter {}", *formatStr);
                }
                else if (string_utils::iequals(key, "pattern"))
                {
                    auto patternStr = value.asString();
                    if (!patternStr)
                        raiseConfigurationError("pattern: expected string");

                    setPattern(std::move(*patternStr));
                }
            }
        }

        void setFormatter(std::shared_ptr<Formatter> formatter)
        {
            m_formatter = std::move(formatter);
            configureFormatter(m_formatter);
        }

        std::shared_ptr<Formatter> formatter() const
        {
            return m_formatter;
        }

    private:
        std::shared_ptr<Formatter> m_formatter;

        template <typename Formatter, typename... Args>
        void createFormatter(Args&&... args)
        {
            setFormatter(std::make_shared<Formatter>(std::forward<Args>(args)...));
        }

        virtual void configureFormatter(const std::shared_ptr<Formatter>&) { }

    protected:
        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out)
        {
            m_formatter->format(name, level, buffer, out);
        }
    };
}
