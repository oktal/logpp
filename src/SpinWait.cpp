#include "SpinWait.h"
#include "logpp/core/config.h"

#include <chrono>
#include <limits>
#include <stdexcept>
#include <thread>

#if defined(LOGPP_PLATFORM_WINDOWS)
#define NOMINMAX
#include <Windows.h>
#endif

namespace logpp
{

    int32_t SpinWait::count() const
    {
        return m_count;
    }

    bool SpinWait::nextSpinWillYield() const
    {
        return m_count > YIELD_THRESHOLD;
    }

    void SpinWait::spinOnce()
    {
        if (nextSpinWillYield())
        {
            auto num = m_count >= YIELD_THRESHOLD ? m_count - 10 : m_count;
            if (num % SLEEP_1_EVERY_HOW_MANY_TIMES == SLEEP_1_EVERY_HOW_MANY_TIMES - 1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else
            {
                if (num % SLEEP_0_EVERY_HOW_MANY_TIMES == SLEEP_0_EVERY_HOW_MANY_TIMES - 1)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(0));
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        }
        else
        {
            spinWaitInternal(4 << m_count);
        }

        m_count = m_count == std::numeric_limits<int32_t>::max() ? YIELD_THRESHOLD : m_count + 1;
    }

    void SpinWait::reset()
    {
        m_count = 0;
    }

    void SpinWait::spinUntil(const std::function<bool()>& condition)
    {
        spinUntil(condition, -1);
    }

    bool SpinWait::spinUntil(const std::function<bool()>& condition, int32_t millisecondsTimeout)
    {
        if (millisecondsTimeout < -1)
            throw std::out_of_range("millisecondsTimeout must be > 0");

        if (!condition)
            throw std::invalid_argument("condition function is nullptr");

        uint32_t num = 0;
        if (millisecondsTimeout != 0 && millisecondsTimeout != -1)
            num = tickCount();

        SpinWait spinWait;
        while (!condition())
        {
            if (millisecondsTimeout == 0)
                return false;

            spinWait.spinOnce();

            if (millisecondsTimeout != 1 && spinWait.nextSpinWillYield() && int64_t(millisecondsTimeout) <= int64_t((uint64_t)(tickCount() - num)))
                return false;
        }

        return true;
    }

    void SpinWait::spinWaitInternal(std::int32_t iterationCount)
    {
        for (int i = 0; i < iterationCount; i++)
            yieldProcessor();
    }

    void SpinWait::yieldProcessor()
    {
        std::this_thread::yield();
    }

    uint32_t SpinWait::tickCount()
    {
#if defined(LOGPP_PLATFORM_WINDOWS)
        return static_cast<uint32_t>(GetTickCount());
#elif defined(LOGPP_PLATFORM_LINUX)
#if CLOCK_MONOTONIC
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return static_cast<uint32_t>((ts.tv_sec * 1000) + (ts.tv_nsec / 1000));
#else
        struct timeval tv;
        gettimeofday(&tv, 0);
        return static_cast<uint32_t>((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
#endif
#else
#error Tick count not available on current platform !
#endif
    }
}
