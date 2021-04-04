#pragma once

#include "logpp/core/LoggerRegistry.h"

#include "logpp/queue/AsyncQueuePoller.h"
#include "logpp/queue/BoundedTypedConcurrentAsyncQueue.h"

#include "logpp/sinks/MultiSink.h"
#include "logpp/sinks/Sink.h"
#include "logpp/utils/string.h"

namespace logpp::sink
{
    class AsyncSink : public SinkBase
    {
    public:
        static constexpr std::string_view Name = "Async";

        AsyncSink() = default;

        AsyncSink(std::shared_ptr<IAsyncQueuePoller> queuePoller, std::shared_ptr<sink::Sink> innerSink)
            : m_queuePoller(std::move(queuePoller))
            , m_queue(std::make_shared<BoundedTypedConcurrentAsyncQueue<Entry>>(512))
            , m_innerSink(std::move(innerSink))

        {
            configureQueue(m_queue);
        }

        void activateOptions(const Options& options) override
        {
            auto sinksOptions = options.tryGet("sinks");
            if (!sinksOptions)
                raiseConfigurationError("expected `sinks` parameter");

            auto sinksArray = sinksOptions->asArray();
            if (!sinksArray)
                raiseConfigurationError("sinks: expected array");

            if (sinksArray->empty())
                raiseConfigurationError("sinks: got empty array");

            auto registry = LoggerRegistry::defaultRegistry();

            std::vector<std::shared_ptr<Sink>> innerSinks;
            for (const auto& sinkName : *sinksArray)
            {
                auto sink = registry.findSink(sinkName);
                if (sink == nullptr)
                    raiseConfigurationError("sinks: unknown sink `{}`", sinkName);

                innerSinks.push_back(std::move(sink));
            }
            auto innerSink = innerSinks.size() == 1 ? innerSinks[0] : std::make_shared<MultiSink>(std::move(innerSinks));

            std::shared_ptr<ITypedAsyncQueue<Entry>> queue;
            auto queueOptions = options.tryGet("queue");
            if (queueOptions)
            {
                auto queueOptionsDict = queueOptions->asDict();
                if (!queueOptionsDict)
                    raiseConfigurationError("queue: expected table");

                auto queueTypeIt = queueOptionsDict->find("type");
                if (queueTypeIt == std::end(*queueOptionsDict))
                    raiseConfigurationError("queue: expected `type` parameter");

                if (string_utils::iequals(queueTypeIt->second, "bounded"))
                {
                    auto queueSizeIt = queueOptionsDict->find("size");
                    size_t queueSize = 512;
                    if (queueSizeIt != std::end(*queueOptionsDict))
                    {
                        auto size = string_utils::parseSize(queueSizeIt->second);
                        if (!size)
                            raiseConfigurationError("queue: invalid size `{}`", queueSizeIt->second);

                        queueSize = *size;
                    }

                    queue = std::make_shared<BoundedTypedConcurrentAsyncQueue<Entry>>(queueSize);
                }
                else
                {
                    raiseConfigurationError("queue: unknown type `{}`", queueTypeIt->second);
                }
            }

            if (!queue)
                raiseConfigurationError("queue: unexpected error");

            configureQueue(queue);

            m_queue     = queue;
            m_innerSink = std::move(innerSink);

            m_queuePoller = AsyncQueuePoller::create();
            m_queuePoller->addQueue(m_queue);
            m_queuePoller->start();
        }

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override
        {
            m_queue->push(Entry::create(name, level, buffer));
        }

        void start()
        {
            m_queuePoller->addQueue(m_queue);
        }

        void stop()
        {
            m_queuePoller->removeQueue(m_queue);
        }

        std::shared_ptr<Sink> innerSink() const
        {
            return m_innerSink;
        }

    private:
        struct Entry
        {
            static Entry create(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
            {
                return Entry { name, level, buffer };
            }

            std::string_view name;
            LogLevel level;
            EventLogBuffer logBuffer;
        };

        std::shared_ptr<IAsyncQueuePoller> m_queuePoller;
        std::shared_ptr<ITypedAsyncQueue<Entry>> m_queue;
        std::shared_ptr<sink::Sink> m_innerSink;

        void configureQueue(const std::shared_ptr<ITypedAsyncQueue<Entry>>& queue)
        {
            queue->setHandler([=](const Entry& entry) {
                handleEntry(entry);
            });
        }

        void handleEntry(const Entry& entry)
        {
            m_innerSink->sink(entry.name, entry.level, entry.logBuffer);
        }
    };
}
