#pragma once

#include "logpp/format/Formatter.h"
#include "logpp/format/PatternFormatter.h"
#include "logpp/format/LogFmtFormatter.h"

#include "logpp/sinks/Sink.h"

#include <fmt/format.h>

namespace logpp::sink
{

    class FormatSink : public Sink
    {
    public:
        explicit FormatSink(std::shared_ptr<Formatter> formatter)
            : m_formatter(std::move(formatter))
        { }

        void setPattern(std::string pattern)
        {
            createFormatter<PatternFormatter>(std::move(pattern));
        }

        bool activateOptions(const Options& options) override
        {
            for (auto&& [key, value]: options)
            {
                if (string_utils::iequals(key, "format"))
                {
                    auto formatStr = value.asString();
                    if (!formatStr)
                        return false;

                    if (string_utils::iequals(*formatStr, "pattern"))
                    {
                        createFormatter<PatternFormatter>();
                        return true;
                    }
                    else if (string_utils::iequals(*formatStr, "logfmt"))
                    {
                        createFormatter<LogFmtFormatter>();
                        return true;
                    }
                }
                else if (string_utils::iequals(key, "pattern"))
                {
                    auto patternStr = value.asString();
                    if (!patternStr)
                        return false;

                    setPattern(std::move(*patternStr));
                    return true;
                }
            }

            return false;
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

        template<typename Formatter, typename... Args>
        void createFormatter(Args&& ...args)
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