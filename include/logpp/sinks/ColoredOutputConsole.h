#include "logpp/sinks/Sink.h"

#include <cstdio>

namespace logpp::sink
{
    class ColoredOutputConsole : public Sink
    {
    public:
        ColoredOutputConsole()
            : m_stream(stdout)
        {}

        void format(std::string_view name, LogLevel level, EventLogBuffer buffer)
        {
            fmt::memory_buffer formatBuf;
            buffer.formatText(formatBuf);

            ::fwrite(formatBuf.data(), 1, formatBuf.size(), m_stream);
        }

    private:
        FILE *m_stream;
    };
}