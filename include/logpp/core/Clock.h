#pragma once

#include "logpp/date/date.h"
#include "logpp/date/tz.h"

#include <chrono>

namespace logpp
{
    using Clock     = std::chrono::system_clock;
    using TimePoint = typename Clock::time_point;
}
