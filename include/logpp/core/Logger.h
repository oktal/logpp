#pragma once

#include "logpp/core/LogLevel.h"
#include "logpp/sinks/Sink.h"
#include "logpp/core/Traits.h"

namespace logpp
{
    class Logger
    {
    public:
        Logger(std::string name, LogLevel level, std::shared_ptr<sink::Sink> sink)
            : m_name(std::move(name))
            , m_level(level)
            , m_sink(std::move(sink))
        {}

        template<typename Str, typename LogFunc>
        void log(Str text, LogLevel level, LogFunc logFunction)
        {
            using Event = typename LogFunctionTraits<LogFunc>::Event;

            Event event;

            EventLogBuffer buffer;

            auto textOffset = buffer.write(text);
            logFunction(buffer, event);
            buffer.writeEvent(event);

            m_sink->format(level, buffer, textOffset);
        }

        template<typename Str, typename LogFunc>
        void debug(Str text, LogFunc logFunction)
        {
            log(text, LogLevel::Debug, logFunction);
        }

    private:
        std::string m_name;
        LogLevel m_level;

        std::shared_ptr<sink::Sink> m_sink;
    };

}