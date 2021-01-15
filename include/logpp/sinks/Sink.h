#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/core/LogLevel.h"
#include "logpp/core/Offset.h"

#include "logpp/utils/detect.h"

namespace logpp::sink
{
    class Sink
    {
    public:
        virtual bool setOption(std::string key, std::string value) = 0;
        virtual void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) = 0;
    };

    namespace details
    {
        template<typename Sink> using Name = decltype(&Sink::Name);

        template<typename Sink> constexpr bool HasName = logpp::is_detected_v<Name, Sink>;

    }

    namespace concepts
    {
        template<typename T> using Sink = std::integral_constant<
            bool,
            std::is_base_of_v<Sink, T> &&
            details::HasName<T>
        >;

        template<typename T> constexpr bool IsSink = Sink<T>::value;
    }

}