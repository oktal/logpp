#pragma once

#if defined(LOGPP_USE_EXTERNAL_DATE) && LOGPP_USE_EXTERNAL_DATE
#include <date/date.h>
#else
#include "logpp/date/date.h"
#endif
