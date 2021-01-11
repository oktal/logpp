#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class LevelFormatter : public FlagFormatter
    {
    public:
        using OnFormat = std::function<void (LogLevel, fmt::memory_buffer& out)>;

        void setPreFormat(OnFormat preFormat)
        {
            m_preFormat = preFormat;
        }

        void setPostFormat(OnFormat postFormat)
        {
            m_postFormat = postFormat;
        }

        void format(std::string_view, LogLevel level, const EventLogBuffer&, fmt::memory_buffer& out) const override
        {
            if (m_preFormat)
                m_preFormat(level, out);

            auto str = levelString(level);
            out.append(str.data(), str.data() + str.size());

            if (m_postFormat)
                m_postFormat(level, out);
        }

    private:
        OnFormat m_preFormat;
        OnFormat m_postFormat;
    };
}