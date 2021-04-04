#pragma once

#include "logpp/core/LogLevel.h"
#include "logpp/sinks/Sink.h"

namespace logpp::sink
{
    class LevelSink : public SinkBase
    {
    public:
        explicit LevelSink(std::shared_ptr<Sink> inner, LogLevel level)
            : m_inner(std::move(inner))
            , m_level(level)
        {}

        void activateOptions(const Options& options) override
        {
            m_inner->activateOptions(options);
        }

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override
        {
            if (!is(level))
                return;

            m_inner->sink(name, level, buffer);
        }

        bool is(LogLevel level) const
        {
            return static_cast<int>(level) >= static_cast<int>(m_level);
        }

    private:
        LogLevel m_level;
        std::shared_ptr<Sink> m_inner;
    };
}
