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

    template<typename Sink, typename... Args>
    void setDefaultLoggerSinked(std::string name, LogLevel level, Args&& ...args)
    {
        auto sink = std::make_shared<Sink>(std::forward<Args>(args)...);
        auto logger = std::make_shared<Logger>(std::move(name), level, std::move(sink));
        setDefaultLogger(std::move(logger));
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
    void trace(const Str& text, Args&&... args)
    {
        log(LogLevel::Trace, text, std::forward<Args>(args)...);
    }

    template<typename Str, typename... Args>
    void debug(const Str& text, Args&&... args)
    {
        log(LogLevel::Debug, text, std::forward<Args>(args)...);
    }

    template<typename Str, typename... Args>
    void info(const Str& text, Args&&... args)
    {
        log(LogLevel::Info, text, std::forward<Args>(args)...);
    }

    template<typename Str, typename... Args>
    void warn(const Str& text, Args&&... args)
    {
        log(LogLevel::Warning, text, std::forward<Args>(args)...);
    }

    template<typename Str, typename... Args>
    void error(const Str& text, Args&&... args)
    {
        log(LogLevel::Error, text, std::forward<Args>(args)...);
    }

    inline void setLevel(LogLevel level)
    {
        defaultLogger()->setLevel(level);
    }

    inline std::shared_ptr<Logger> getLogger(std::string_view name)
    {
        return defaultRegistry().get(name);
    }

    template<typename Sink, typename... SinkArgs>
    std::shared_ptr<Logger> create(std::string name, LogLevel level, SinkArgs&& ...args)
    {
        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        return std::make_shared<Logger>(std::move(name), level, std::move(sink));
    }
}