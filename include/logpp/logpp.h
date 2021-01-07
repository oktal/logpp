#pragma once

#include "logpp/core/Logger.h"
#include "logpp/core/LoggerRegistry.h"

namespace logpp
{

    inline LoggerRegistry& defaultRegistry()
    {
        return LoggerRegistry::defaultRegistry();
    }

    inline void setDefaultLogger(std::shared_ptr<Logger> logger)
    {
        defaultRegistry().setDefaultLogger(std::move(logger));
    }

    inline std::shared_ptr<Logger> defaultLogger()
    {
        return defaultRegistry().defaultLogger();
    }

    template<typename Str, typename... Args>
    void log(LogLevel level, Str text, Args&&... args)
    {
        defaultLogger()->log(text, level, std::forward<Args>(args)...);
    }

    template<typename Str, typename... Args>
    void debug(Str text, Args&&... args)
    {
        log(LogLevel::Debug, text, std::forward<Args>(args)...);
    }

}