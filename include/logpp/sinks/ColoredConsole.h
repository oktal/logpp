#include "logpp/sinks/FormatSink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/format/flag/LevelFormatter.h"

#include "logpp/utils/rang.hpp"

#include <array>
#include <iostream>
#include <mutex>

namespace logpp::sink
{
    class ColoredConsole : public FormatSink
    {
    public:
        explicit ColoredConsole(std::ostream& os)
            : ColoredConsole(os, std::make_shared<PatternFormatter>("%+"))
        { }

        explicit ColoredConsole(std::ostream& os, const std::shared_ptr<Formatter>& formatter)
            : FormatSink(formatter)
            , m_os(os)
        {
            setColor(LogLevel::Trace, rang::fg::gray);
            setColor(LogLevel::Debug, rang::fg::green);
            setColor(LogLevel::Info, rang::fg::cyan);
            setColor(LogLevel::Warning, rang::fg::yellow);
            setColor(LogLevel::Error, rang::fg::red);
        }

        void setColor(LogLevel level, rang::fg color)
        {
            m_colors[static_cast<size_t>(level)] = color;
        }

        rang::fg getColor(LogLevel level) const
        {
            return m_colors[static_cast<size_t>(level)];
        }

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
        {
            fmt::memory_buffer formatBuf;
            format(name, level, buffer, formatBuf);

            auto color = getColor(level);

            std::lock_guard guard(m_mutex);
            m_os << color << std::string_view(formatBuf.data(), formatBuf.size());
            m_os.put('\n');
        }

    private:
        std::mutex m_mutex;
        std::ostream& m_os;
        std::array<rang::fg, 5> m_colors;
    };

    class ColoredOutputConsole : public ColoredConsole
    {
    public:
        static constexpr std::string_view Name = "ColoredOutputConsole";

        ColoredOutputConsole()
            : ColoredConsole(std::cout)
        { }

        explicit ColoredOutputConsole(std::shared_ptr<Formatter> formatter)
            : ColoredConsole(std::cout, std::move(formatter))
        { }
    };

    class ColoredErrorConsole : public ColoredConsole
    {
    public:
        static constexpr std::string_view Name = "ColoredErrorConsole";

        ColoredErrorConsole()
            : ColoredConsole(std::cerr)
        { }

        ColoredErrorConsole(std::shared_ptr<Formatter> formatter)
            : ColoredConsole(std::cerr, std::move(formatter))
        { }
    };
}
