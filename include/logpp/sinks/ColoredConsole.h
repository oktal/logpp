#include "logpp/sinks/Sink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/format/flag/LevelFormatter.h"

#include <cstdio>

namespace logpp::sink
{
    class ColoredConsole : public Sink
    {
    public:
        ColoredConsole(FILE* fp)
            : m_file(fp)
            , m_formatter(std::make_shared<PatternFormatter>("%+"))
        {
            configureFormatter(m_formatter);

            setColor(LogLevel::Trace, FgWhite);
            setColor(LogLevel::Debug, FgGreen);
            setColor(LogLevel::Info, FgCyan);
            setColor(LogLevel::Warning, FgYellow);
            setColor(LogLevel::Error, FgRed);
        }

        bool setOption(std::string key, std::string value) override
        {
            if (key == "pattern")
            {
                setPattern(std::move(value));
                return true;
            }

            return false;
        }

        void setPattern(std::string pattern)
        {
            m_formatter->setPattern(std::move(pattern));
            configureFormatter(m_formatter);
        }

        void setColor(LogLevel level, std::string_view code)
        {
            m_colors[static_cast<size_t>(level)] = code;
        }

        std::string_view getColor(LogLevel level) const
        {
            return m_colors[static_cast<size_t>(level)];
        }

        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
        {
            fmt::memory_buffer formatBuf;
            m_formatter->format(name, level, buffer, formatBuf);

            ::fwrite(formatBuf.data(), 1, formatBuf.size(), m_file);
            ::fputc('\n', m_file);
        }
    private:
        FILE *m_file;
        std::shared_ptr<PatternFormatter> m_formatter;

        std::array<std::string_view, 5> m_colors;

        static constexpr std::string_view Bold = "\033[1m";
        static constexpr std::string_view Reset = "\033[m";

        static constexpr std::string_view FgBlack = "\033[30m";
        static constexpr std::string_view FgRed = "\033[31m";
        static constexpr std::string_view FgGreen = "\033[32m";
        static constexpr std::string_view FgYellow = "\033[33m";
        static constexpr std::string_view FgBlue = "\033[34m";
        static constexpr std::string_view FgMagenta = "\033[35m";
        static constexpr std::string_view FgCyan = "\033[36m";
        static constexpr std::string_view FgWhite = "\033[37m";

        void configureFormatter(const std::shared_ptr<PatternFormatter>& formatter)
        {
            auto levelFormatter = formatter->getFlagFormatter<LevelFormatter>();
            if (levelFormatter)
            {
                levelFormatter->setPreFormat([&](LogLevel level, fmt::memory_buffer& out) {
                    auto color = getColor(level);
                    fmt::format_to(out, "{}{}", Bold, color);
                });
                levelFormatter->setPostFormat([&](LogLevel, fmt::memory_buffer& out) {
                    out.append(Reset.data(), Reset.data() + Reset.size());
                });
            }
        }
    };

    class ColoredOutputConsole : public ColoredConsole
    {
    public:
        static constexpr std::string_view Name = "ColoredOutputConsole";

        ColoredOutputConsole()
            : ColoredConsole(stdout)
        {}
    };

    class ColoredErrorConsole : public ColoredConsole
    {
    public:
        static constexpr std::string_view Name = "ColoredErrorConsole";

        ColoredErrorConsole()
            : ColoredConsole(stderr)
        {}
    };
}