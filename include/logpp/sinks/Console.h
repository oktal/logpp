#include "logpp/format/PatternFormatter.h"
#include "logpp/sinks/FormatSink.h"

#include <iostream>
#include <mutex>

namespace logpp::sink
{
    class Console : public FormatSink
    {
    public:
        explicit Console(std::ostream& os)
            : Console(os, std::make_shared<PatternFormatter>("%+"))
        { }

        explicit Console(std::ostream& os, const std::shared_ptr<Formatter>& formatter)
            : FormatSink(formatter)
            , m_os(os)
        {
        }

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override
        {
            fmt::memory_buffer formatBuf;
            format(name, level, buffer, formatBuf);

            std::lock_guard guard(m_mutex);
            m_os.write(formatBuf.data(), formatBuf.size());
            m_os.put('\n');
        }

    private:
        std::mutex m_mutex;
        std::ostream& m_os;
    };

    class OutputConsole : public Console
    {
    public:
        static constexpr std::string_view Name = "OutputConsole";

        OutputConsole()
            : Console(std::cout)
        { }

        explicit OutputConsole(std::shared_ptr<Formatter> formatter)
            : Console(std::cout, std::move(formatter))
        { }
    };

    class ErrorConsole : public Console
    {
    public:
        static constexpr std::string_view Name = "ErrorConsole";

        ErrorConsole()
            : Console(std::cerr)
        { }

        ErrorConsole(std::shared_ptr<Formatter> formatter)
            : Console(std::cerr, std::move(formatter))
        { }
    };
}
