#include "logpp/sinks/Sink.h"

#include "logpp/format/PatternFormatter.h"

#include <cstdio>

namespace logpp::sink
{
    class ColoredOutputConsole : public Sink
    {
    public:
        ColoredOutputConsole()
            : m_stream(stdout)
            , m_formatter(std::make_shared<PatternFormatter>("%+"))
        {}

        void setPattern(std::string pattern)
        {
            m_formatter = std::make_shared<PatternFormatter>(std::move(pattern));
        }

        void setFormatter(std::shared_ptr<Formatter> formatter)
        {
            m_formatter = std::move(formatter);
        }

        void format(std::string_view name, LogLevel level, EventLogBuffer buffer)
        {
            fmt::memory_buffer formatBuf;
            m_formatter->format(name, level, buffer, formatBuf);

            ::fwrite(formatBuf.data(), 1, formatBuf.size(), m_stream);
            ::fputc('\n', m_stream);
        }

    private:
        FILE *m_stream;
        std::shared_ptr<Formatter> m_formatter;
    };
}