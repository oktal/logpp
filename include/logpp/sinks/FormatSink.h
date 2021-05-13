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

        void activateOptions(const Options& options) override
        {
            for (auto&& [key, value] : options)
            {
                if (string_utils::iequals(key, "format"))
                {
                    if (auto formatStr = value.asString())
                    {
                        if (string_utils::iequals(*formatStr, "pattern"))
                            setFormatter(std::make_shared<PatternFormatter>());
                        else if (string_utils::iequals(*formatStr, "logfmt"))
                            setFormatter(std::make_shared<LogFmtFormatter>());
                        else
                            raiseConfigurationError("format: unknown formatter {}", *formatStr);
                    }
                    else if (auto opts = value.asDict())
                    {
                        auto typeIt = opts->find("type");
                        if (typeIt == std::end(*opts))
                            raiseConfigurationError("format: missing `type`");

                        auto pattern = parsePatternOr(*opts, "%+");

                        if (string_utils::iequals(typeIt->second, "pattern"))
                            setFormatter(std::make_shared<PatternFormatter>(std::move(pattern)));
                        else if (string_utils::iequals(typeIt->second, "logfmt"))
                            setFormatter(std::make_shared<LogFmtFormatter>(std::move(pattern)));
                        else
                            raiseConfigurationError("format: invalid `type` {}", typeIt->second);
                    }
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

        std::string parsePatternOr(const Options::Dict& opts, std::string defaultValue)
        {
            auto patternIt = opts.find("pattern");
            if (patternIt != std::end(opts))
                return patternIt->second;

            return defaultValue;
        }

        virtual void configureFormatter(const std::shared_ptr<Formatter>&) { }

    protected:
        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& out)
        {
            m_formatter->format(name, level, buffer, out);
        }
    };
}
