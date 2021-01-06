#pragma once

#include <cstdint>
#include <functional>

namespace logpp
{

    class SpinWait
    {
    public:
        SpinWait() = default;

        int32_t count() const;
        bool nextSpinWillYield() const;
        
        void spinOnce();

        void reset();

        static void spinUntil(const std::function< bool() >& condition);
        static bool spinUntil(const std::function< bool() >& condition, int32_t millisecondsTimeout);

    private:
        static constexpr int32_t YIELD_THRESHOLD = 10;
        static constexpr int32_t SLEEP_0_EVERY_HOW_MANY_TIMES = 5;
        static constexpr int32_t SLEEP_1_EVERY_HOW_MANY_TIMES = 20;

        static void spinWaitInternal(int32_t iterationCount);
        static void yieldProcessor();

        static uint32_t tickCount();

        int32_t m_count {};
    };

}