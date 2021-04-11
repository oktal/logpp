#pragma once

#include "logpp/sinks/Sink.h"

namespace logpp::sink
{
    class MultiSink : public Sink
    {
    public:
        using SinkPtr = std::shared_ptr<Sink>;

        explicit MultiSink(std::vector<SinkPtr> sinks)
            : m_innerSinks(std::move(sinks))
        { }

        explicit MultiSink(std::initializer_list<SinkPtr> sinks)
            : m_innerSinks(std::begin(sinks), std::end(sinks))
        { }

        void addSink(SinkPtr sink)
        {
            m_innerSinks.push_back(std::move(sink));
        }

        void activateOptions(const Options&) override
        {
        }

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override
        {
            for (auto& sink : m_innerSinks)
            {
                sink->sink(name, level, buffer);
            }
        }

    private:
        std::vector<SinkPtr> m_innerSinks;
    };
}
