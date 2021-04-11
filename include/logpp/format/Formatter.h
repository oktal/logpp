#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/core/LogLevel.h"

#include <fmt/format.h>

namespace logpp
{
    class Formatter
    {
    public:
        using OnFormat = std::function<void(std::string_view, LogLevel level, const EventLogBuffer&, fmt::memory_buffer&)>;

        virtual ~Formatter() = default;

        void setPreFormat(OnFormat onFormat)
        {
            m_preFormat = onFormat;
        }

        void setPostFormat(OnFormat onFormat)
        {
            m_postFormat = onFormat;
        }

        void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& dest)
        {
            if (m_preFormat)
                std::invoke(m_preFormat, name, level, buffer, dest);

            doFormat(name, level, buffer, dest);

            if (m_postFormat)
                std::invoke(m_postFormat, name, level, buffer, dest);
        }

    private:
        virtual void doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& dest) const = 0;

        OnFormat m_preFormat;
        OnFormat m_postFormat;
    };

    namespace concepts
    {
        template <typename T>
        using Formatter = std::integral_constant<
            bool,
            std::is_base_of_v<Formatter, T>>;

        template <typename T>
        constexpr bool IsFormatter = Formatter<T>::value;
    }
}