#pragma once

#include "logpp/core/Logger.h"
#include "logpp/core/LoggerRegistry.h"

#include "logpp/format/Formatter.h"

#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/FormatSink.h"

namespace logpp
{
#define LOGPP_LOG(str, level, ...)                                           \
    do                                                                       \
    {                                                                        \
        logpp::log(level, str, logpp::SourceLocation { __FILE__, __LINE__ }, \
                   __VA_ARGS__);                                             \
    } while (0)

#define LOGPP_TRACE(str, ...) \
    LOGPP_LOG(str, logpp::LogLevel::Trace, __VA_ARGS__)

#define LOGPP_DEBUG(str, ...) \
    LOGPP_LOG(str, logpp::LogLevel::Debug, __VA_ARGS__)

#define LOGPP_INFO(str, ...) \
    LOGPP_LOG(str, logpp::LogLevel::Info, __VA_ARGS__)

#define LOGPP_WARN(str, ...) \
    LOGPP_LOG(str, logpp::LogLevel::Warning, __VA_ARGS__)

#define LOGPP_ERROR(str, ...) \
    LOGPP_LOG(str, logpp::LogLevel::Error, __VA_ARGS__)

#define LOGPP_FORMAT(str, ...) \
    logpp::format(str, __VA_ARGS__)

#define LOGPP_FIELD(key, val) \
    logpp::field(key, val)

    namespace details
    {
        template <typename T>
        static constexpr auto nameOf() noexcept
        {
            std::string_view name = "Error: unsupported compiler", prefix, suffix;
#ifdef LOGPP_COMPILER_CLANG
            name   = __PRETTY_FUNCTION__;
            prefix = "auto logpp::details::nameOf() [T = ";
            suffix = "]";
#elif defined(LOGPP_COMPILER_GCC)
            name   = __PRETTY_FUNCTION__;
            prefix = "constexpr auto logpp::details::nameOf() [with T = ";
            suffix = "]";
#elif defined(LOGPP_COMPILER_MSVC)
            name   = __FUNCSIG__;
            prefix = "auto __cdecl logpp::details::nameOf<";
            suffix = ">(void) noexcept";
#endif
            name.remove_prefix(prefix.size());
            name.remove_suffix(suffix.size());
            return name;
        }
    }

    inline LoggerRegistry& defaultRegistry()
    {
        return LoggerRegistry::defaultRegistry();
    }

    inline void setDefaultLogger(std::shared_ptr<Logger> logger)
    {
        defaultRegistry().setDefaultLogger(std::move(logger));
    }

    inline void setDefaultLoggerFunc(LoggerRegistry::LoggerFactory factory)
    {
        defaultRegistry().setDefaultLoggerFunc(std::move(factory));
    }

    inline std::shared_ptr<Logger> defaultLogger()
    {
        return defaultRegistry().defaultLogger();
    }

    template <typename Sink, typename... Args>
    std::shared_ptr<Sink> setDefaultLoggerSinked(std::string name, LogLevel level, Args&&... args)
    {
        auto sink   = std::make_shared<Sink>(std::forward<Args>(args)...);
        auto logger = std::make_shared<Logger>(std::move(name), level, sink);
        setDefaultLogger(std::move(logger));

        return sink;
    }

    template <typename Str, typename... Args>
    void log(const std::shared_ptr<Logger>& logger, LogLevel level, Str text, Args&&... args)
    {
        if (!logger)
            return;

        logger->log(text, level, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void trace(const std::shared_ptr<Logger>& logger, const Str& text, Args&&... args)
    {
        log(logger, LogLevel::Trace, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void debug(const std::shared_ptr<Logger>& logger, const Str& text, Args&&... args)
    {
        log(logger, LogLevel::Debug, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void info(const std::shared_ptr<Logger>& logger, const Str& text, Args&&... args)
    {
        log(logger, LogLevel::Info, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void warn(const std::shared_ptr<Logger>& logger, const Str& text, Args&&... args)
    {
        log(logger, LogLevel::Warning, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void error(const std::shared_ptr<Logger>& logger, const Str& text, Args&&... args)
    {
        log(LogLevel::Error, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void log(LogLevel level, Str text, Args&&... args)
    {
        log(defaultLogger(), level, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void trace(const Str& text, Args&&... args)
    {
        log(LogLevel::Trace, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void debug(const Str& text, Args&&... args)
    {
        log(LogLevel::Debug, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void info(const Str& text, Args&&... args)
    {
        log(LogLevel::Info, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void warn(const Str& text, Args&&... args)
    {
        log(LogLevel::Warning, text, std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    void error(const Str& text, Args&&... args)
    {
        log(LogLevel::Error, text, std::forward<Args>(args)...);
    }

    inline void setLevel(LogLevel level)
    {
        defaultLogger()->setLevel(level);
    }

    template <typename Formatter, typename... Args>
    bool setFormatter(Args&&... args)
    {
        static_assert(concepts::IsFormatter<Formatter>, "Formatter must satisfy the Formatter concept");

        auto logger = defaultLogger();
        auto sink   = logger->sink();

        if (auto asyncSink = std::dynamic_pointer_cast<sink::AsyncSink>(sink))
            sink = asyncSink->innerSink();

        if (auto formatSink = std::dynamic_pointer_cast<sink::FormatSink>(sink))
        {
            formatSink->setFormatter(std::make_shared<Formatter>(std::forward<Args>(args)...));
            return true;
        }

        return false;
    }

    inline std::shared_ptr<Logger> getLogger(std::string_view name)
    {
        return defaultRegistry().get(name);
    }

    template <typename T>
    std::shared_ptr<Logger> getLogger()
    {
        return defaultRegistry().get(details::nameOf<T>());
    }
}
