#pragma once

#include "logpp/core/FormatArgs.h"
#include "logpp/core/LogFieldVisitor.h"
#include "logpp/core/LogLevel.h"

#include "logpp/sinks/Sink.h"

#include <atomic>

namespace logpp
{
    template<typename KeyStr, typename T>
    struct LogField
    {
        using Key = KeyStr;
        using Type = T;

        LogField(KeyStr key, T& value)
            : key(std::move(key))
            , value(value)
        {}

        KeyStr key;
        T& value;
    };

    template<typename KeyStr, typename T>
    LogField<KeyStr, T> field(KeyStr key, T&& value)
    {
        return { std::move(key), value };
    }

    template<size_t N, typename T>
    LogField<StringLiteral, T> field(const char (&key)[N], T&& value)
    {
        return { StringLiteral { key }, value };
    }

    template<typename... Args>
    FormatArgsHolder<std::decay_t<Args>...> format(std::string_view formatStr, Args&& ...args)
    {
        return { formatStr, std::make_tuple(std::forward<Args>(args)...)};
    }

    class Logger
    {
    public:
        Logger(std::string name, LogLevel level, std::shared_ptr<sink::Sink> sink)
            : m_name(std::move(name))
            , m_level(level)
            , m_sink(std::move(sink))
        {}

        template<typename Str, typename... Fields>
        void log(const Str& text, LogLevel level, Fields&&... fields)
        {
            if (!is(level))
                return;

            EventLogBuffer buffer;

            buffer.writeTime(Clock::now());
            buffer.writeText(text);
            buffer.writeFields(std::forward<Fields>(fields)...);

            m_sink->sink(name(), level, buffer);
        }

        template<typename Str, typename... Args>
        void trace(Str text, Args&&... args)
        {
            log(text, LogLevel::Trace, std::forward<Args>(args)...);
        }

        template<typename Str, typename... Args>
        void debug(Str text, Args&&... args)
        {
            log(text, LogLevel::Debug, std::forward<Args>(args)...);
        }

        template<typename Str, typename... Args>
        void info(Str text, Args&&... args)
        {
            log(text, LogLevel::Info, std::forward<Args>(args)...);
        }

        template<typename Str, typename... Args>
        void warn(Str text, Args&&... args)
        {
            log(text, LogLevel::Warning, std::forward<Args>(args)...);
        }

        template<typename Str, typename... Args>
        void error(Str text, Args&&... args)
        {
            log(text, LogLevel::Error, std::forward<Args>(args)...);
        }

        std::shared_ptr<sink::Sink> sink() const
        {
            return m_sink;
        }

        std::string_view name() const
        {
            return m_name;
        }

        LogLevel level() const
        {
            return m_level.load(std::memory_order_relaxed);
        }

        void setLevel(LogLevel level)
        {
            m_level.store(level, std::memory_order_relaxed);
        }

        bool is(LogLevel lvl) const
        {
            return static_cast<int>(lvl) >= static_cast<int>(level());
        }

    private:
        std::string m_name;
        std::atomic<LogLevel> m_level;

        std::shared_ptr<sink::Sink> m_sink;
    };
}