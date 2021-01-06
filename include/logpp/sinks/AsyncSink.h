#pragma once

#include "logpp/queue/IAsyncQueuePoller.h"
#include "logpp/queue/ITypedAsyncQueue.h"
#include "logpp/queue/BoundedTypedConcurrentAsyncQueue.h"
#include "logpp/sinks/Sink.h"

#include <chrono>

namespace logpp::sink
{
    class AsyncSink : public Sink
    {
    public:
        AsyncSink(std::shared_ptr<IAsyncQueuePoller> queuePoller, std::shared_ptr<sink::Sink> innerSink)
            : m_queuePoller(std::move(queuePoller))
            , m_queue(std::make_shared<BoundedTypedConcurrentAsyncQueue<Entry>>(512))
            , m_innerSink(std::move(innerSink))

        {
            m_queue->setHandler([=](const Entry& entry)
            {
                handleEntry(entry);
            });
        }

        void format(LogLevel level, EventLogBuffer buffer, StringOffset text)
        {
            m_queue->push(Entry::create(level, text, buffer));
        }

        void start()
        {
            m_queuePoller->addQueue(m_queue);
        }

        void stop()
        {
            m_queuePoller->removeQueue(m_queue);
        }

    private:
        struct Entry
        {
            static Entry create(LogLevel level, StringOffset textOffset, const EventLogBuffer& buffer)
            {
                return Entry{level, textOffset, buffer};
            }

            LogLevel level;
            StringOffset textOffset;
            EventLogBuffer logBuffer;
        };

        std::shared_ptr<IAsyncQueuePoller> m_queuePoller;
        std::shared_ptr<ITypedAsyncQueue<Entry>> m_queue;
        std::shared_ptr<sink::Sink> m_innerSink;

        void handleEntry(const Entry& entry)
        {
            m_innerSink->format(entry.level, entry.logBuffer, entry.textOffset);
        }
    };
}