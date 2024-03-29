#pragma once

#include "logpp/core/config.h"

#include "logpp/queue/ITypedAsyncQueue.h"
// Disable structure padding warning C4324
#if defined(LOGPP_COMPILER_MSVC)
  #pragma warning( push )
  #pragma warning( disable: 4324 )
#endif
#include "logpp/queue/impl/MPMCQueue.h"
#if defined (LOGPP_COMPILER_MSVC)
  #pragma warning( pop )
#endif

#include <climits>

namespace logpp
{

    template <typename Entry>
    class BoundedTypedConcurrentAsyncQueue : public ITypedAsyncQueue<Entry>
    {
    public:
        using Handler = typename ITypedAsyncQueue<Entry>::Handler;

        explicit BoundedTypedConcurrentAsyncQueue(size_t size)
            : m_queue(roundNextPowerOfTwo(size))
        { }

        size_t poll() override
        {
            size_t totalCount = 0;

            for (;;)
            {
                Entry entry;
                auto res = m_queue.try_pop(entry);
                if (!res)
                    break;

                handleEntry(entry);
                ++totalCount;
            }

            return totalCount;
        }

        size_t pollOne() override
        {
            Entry entry;
            if (m_queue.try_pop(entry))
            {
                handleEntry(entry);
                return 1;
            }

            return 0;
        }

        void setHandler(const Handler& handler) override
        {
            m_handler = handler;
        }

        void push(const Entry& entry) override
        {
            m_queue.push(entry);
        }

        void push(Entry&& entry) override
        {
            m_queue.push(std::move(entry));
        }

    private:
        using Queue = rigtorp::MPMCQueue<Entry>;
        Queue m_queue;

        Handler m_handler;

        void handleEntry(const Entry& entry)
        {
            if (m_handler)
                m_handler(entry);
        }

        static size_t roundNextPowerOfTwo(size_t value)
        {
            if (value == 0)
                return 1;

            if (isPowerOfTwo(value))
                return value;

            --value;
            for (size_t i = 1; i < sizeof(size_t) * CHAR_BIT; i *= 2)
            {
                value |= value >> i;
            }

            return value + 1;
        }

        static constexpr bool isPowerOfTwo(size_t value)
        {
            return value != 0 && (value & (value - 1)) == 0;
        }
    };
}
