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

        bool setOption(std::string key, std::string value) override
        {
            if (key == "format")
            {
                if (value == "pattern")
                {
                    createFormatter<PatternFormatter>();
                    return true;
                }
                if (value == "logfmt")
                {
                    createFormatter<LogFmtFormatter>();
                    return true;
                }
            }
            if (key == "pattern")
            {
                setPattern(std::move(value));
                return true;
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