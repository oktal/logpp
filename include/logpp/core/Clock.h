#pragma once

#include "logpp/core/Date.h"

#include <chrono>

namespace logpp
{
    using Clock     = std::chrono::system_clock;
    using TimePoint = typename Clock::time_point;
}
