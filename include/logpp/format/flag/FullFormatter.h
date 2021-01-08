#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class FullFormatter : public FlagFormatter
    {
    public:
        using OnLevel = std::function<void (LogLevel, fmt::memory_buffer& out)>;

        struct LevelFormat
        {
            fmt::memory_buffer& buffer;
            LogLevel level;
            const OnLevel& preLevel;
            const OnLevel& postLevel;
        };

        void setPreLevelFormat(OnLevel preLevelFormat)
        {
            m_preLevelFormat = preLevelFormat;
        }

        void setPostLevelFormat(OnLevel postLevelFormat)
        {
            m_postLevelFormat = postLevelFormat;
        }

        void format(std::string_view name, LogLevel level, const std::tm* time, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            fmt::format_to(out, "{:04}-{:02}-{:02} {:02}:{:02}:{:02} [{}]",
                time->tm_year + 1900,
                time->tm_mon + 1,
                time->tm_mday,
                time->tm_hour,
                time->tm_min,
                time->tm_sec,
                LevelFormat { out, level, m_preLevelFormat, m_postLevelFormat }
            );

            if (!name.empty())
                fmt::format_to(out, " {} -", name);

            out.push_back(' ');
            buffer.formatText(out);
        }

    private:
        OnLevel m_preLevelFormat;
        OnLevel m_postLevelFormat;
    };
}

template<>
struct fmt::formatter<logpp::FullFormatter::LevelFormat>
{
    constexpr auto parse(fmt::format_parse_context& context)
    {
        return context.begin();
    }

    template<typename FormatContext>
    auto format(const logpp::FullFormatter::LevelFormat& format, FormatContext& ctx)
    {
        if (format.preLevel)
            format.preLevel(format.level, format.buffer);

        fmt::format_to(ctx.out(), "{}", logpp::levelString(format.level));

        if (format.postLevel)
            format.postLevel(format.level, format.buffer);

        return ctx.out();
    }
};