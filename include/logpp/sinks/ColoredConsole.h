#include "logpp/sinks/FormatSink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/format/flag/LevelFormatter.h"

#include "logpp/utils/rang.hpp"

#include <array>
#include <iostream>
#include <mutex>

namespace logpp::sink
{

#define COLORS                \
    COLOR("black", black)     \
    COLOR("red", red)         \
    COLOR("green", green)     \
    COLOR("yellow", yellow)   \
    COLOR("blue", blue)       \
    COLOR("magenta", magenta) \
    COLOR("cyan", cyan)       \
    COLOR("gray", gray)

    class Theme
    {
    public:
        struct Color
        {
            std::optional<rang::fg> fg;
            // :NestedConfigurationKey
#if 0
            std::optional<rang::bg> bg;
            std::optional<rang::style> style;
#endif
        };

        void setFgColor(LogLevel level, rang::fg color)
        {
            m_colors[static_cast<size_t>(level)].fg = color;
        }

        std::optional<rang::fg> getFgColor(LogLevel level) const
        {
            return m_colors[static_cast<size_t>(level)].fg;
        }

        // TODO:
        // :NestedConfigurationKey
        // We currently do not support nested keys in sink configuration options. For example, we would like
        // to support configuring the foreground, background colors and style for a particular level, e.g
        // [sinks.console.options.theme]
        //     trace = "gray"
        //     error = { fg = "red", bg = "white", style = "blink" }
        // We could extend the `Options` `Dict` type to support nested keys encoded as a path, e.g
        // `error.fg=red`, `error.bg=white`, `error.style=blink`
        static Theme parseConfig(const Options::Dict& themeOptions)
        {
            Theme theme;

            for (auto&& [levelStr, colorStr] : themeOptions)
            {
                auto level = parseLevel(levelStr);
                if (!level)
                    SinkBase::raiseConfigurationError("theme: invalid `level` {}", levelStr);

                auto color = parseColor<rang::fg>(colorStr);
                if (!color)
                    SinkBase::raiseConfigurationError("theme: invalid `color` {}", colorStr);

                theme.setFgColor(*level, *color);
            }

            return theme;
        }

        void applyTo(std::ostream& os, LogLevel level, std::string_view str)
        {
            auto color = getFgColor(level);

            if (color)
                os << *color << str << rang::fg::reset;
            else
                os << str;
        }

    private:
        std::array<Color, 5> m_colors;

        template <typename T>
        static std::optional<T> parseColor(std::string_view colorString)
        {
#define COLOR(name, ident)                        \
    if (string_utils::iequals(colorString, name)) \
        return T::ident;
            COLORS
#undef COLOR

            return std::nullopt;
        }
    };

    struct DefaultTheme : Theme
    {
        DefaultTheme()
        {
            setFgColor(LogLevel::Trace, rang::fg::gray);
            setFgColor(LogLevel::Debug, rang::fg::green);
            setFgColor(LogLevel::Info, rang::fg::cyan);
            setFgColor(LogLevel::Warning, rang::fg::yellow);
            setFgColor(LogLevel::Error, rang::fg::red);
        }
    };

    class ColoredConsole : public FormatSink
    {
    public:
        explicit ColoredConsole(std::ostream& os)
            : ColoredConsole(os, std::make_shared<PatternFormatter>("%+"))
        { }

        explicit ColoredConsole(std::ostream& os, const std::shared_ptr<Formatter>& formatter)
            : FormatSink(formatter)
            , m_os(os)
            , m_theme(DefaultTheme{})
        {
        }

        void setTheme(Theme theme)
        {
            m_theme = theme;
        }

        Theme theme() const
        {
            return m_theme;
        }

        void activateOptions(const Options& options) override
        {
            FormatSink::activateOptions(options);

            if (auto theme = options.tryGet("theme"))
            {
                auto themeDict = theme->asDict();
                if (!themeDict)
                    raiseConfigurationError("invalid `theme`, expected dictionary");

                m_theme = Theme::parseConfig(*themeDict);
            }
        }

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override
        {
            fmt::memory_buffer formatBuf;
            format(name, level, buffer, formatBuf);

            std::lock_guard guard(m_mutex);
            m_theme.applyTo(m_os, level, std::string_view(formatBuf.data(), formatBuf.size()));
            m_os.put('\n');
        }

    private:
        std::mutex m_mutex;
        std::ostream& m_os;
        Theme m_theme;
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
