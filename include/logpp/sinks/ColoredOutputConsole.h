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
            auto text = buffer.text();
            ::fwrite(text.data(), 1, text.size(), m_stream);
        }

    private:
        FILE *m_stream;
    };
}