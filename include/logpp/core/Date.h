#pragma once

#if defined(LOGPP_USE_EXTERNAL_DATE) && LOGPP_USE_EXTERNAL_DATE
#include <date/date.h>
#include <date/tz.h>
#else
#include "logpp/date/date.h"
#include "logpp/date/tz.h"
#endif
