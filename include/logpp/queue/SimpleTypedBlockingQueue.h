#pragma once

#include "logpp/queue/ITypedAsyncQueue.h"

#include <deque>
#include <mutex>

namespace logpp
{
    template <typename Entry>
    class SimpleBlockingQueue : public ITypedAsyncQueue<Entry>
    {
    public:
        using Handler = typename ITypedAsyncQueue<Entry>::Handler;

        size_t poll() override
        {
            size_t totalCount = 0;

            std::scoped_lock<std::mutex> guard { m_mutex };

            for (;;)
            {
                if (m_queue.empty())
                    break;

                auto& entry = m_queue.front();
                handleEntry(entry);
                m_queue.pop_front();
                ++totalCount;
            }

            return totalCount;
        }

        size_t pollOne() override
        {
            std::scoped_lock<std::mutex> guard { m_mutex };
            if (m_queue.empty())
                return 0;

            auto& entry = m_queue.front();
            handleEntry(entry);
            m_queue.pop_front();
            return 1;
        }

        void setHandler(const Handler& handler) override
        {
            m_handler = handler;
        }

        void push(const Entry& entry) override
        {
            std::scoped_lock<std::mutex> guard { m_mutex };
            m_queue.push_back(entry);
        }

        void push(Entry&& entry) override
        {
            std::scoped_lock<std::mutex> guard { m_mutex };
            m_queue.push_back(std::move(entry));
        }

    private:
        std::mutex m_mutex;
        std::deque<Entry> m_queue;

        Handler m_handler;

        void handleEntry(const Entry& entry)
        {
            if (m_handler)
                m_handler(entry);
        }
    };
}
