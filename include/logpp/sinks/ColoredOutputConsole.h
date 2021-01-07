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

        void format(std::string_view name, LogLevel level, EventLogBuffer buffer, StringOffset text)
        {
            LogBufferView view { buffer };
            auto message = text.get(view);
            ::fwrite(message.data(), 1, message.size(), m_stream);
        }

    private:
        FILE *m_stream;
    };
}