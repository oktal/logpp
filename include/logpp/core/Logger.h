#pragma once

#include "logpp/core/FormatArgs.h"
#include "logpp/core/LogLevel.h"
#include "logpp/core/LogVisitor.h"

#include "logpp/sinks/Sink.h"

namespace logpp
{
    template<typename KeyStr, typename T>
    struct StructuredDataWrapper
    {
        using Key = KeyStr;
        using Type = T;

        StructuredDataWrapper(KeyStr key, T& value)
            : key(key)
            , value(value)
        {}

        KeyStr key;
        T& value;
    };

    template<typename KeyStr, typename T>
    StructuredDataWrapper<KeyStr, T> data(KeyStr key, T&& value)
    {
        return { key, value };
    }

    template<size_t N, typename T>
    StructuredDataWrapper<StringLiteral, T> data(const char (&key)[N], T&& value)
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

        template<typename Str, typename... Args>
        void log(const Str& text, LogLevel level, Args&&... args)
        {
            EventLogBuffer buffer;

            buffer.writeTime(Clock::now());
            buffer.writeText(text);
            buffer.writeData(std::forward<Args>(args)...);

            m_sink->format(name(), level, buffer);
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

        std::string_view name() const
        {
            return m_name;
        }

        LogLevel level() const
        {
            return m_level;
        }

    private:
        std::string m_name;
        LogLevel m_level;

        std::shared_ptr<sink::Sink> m_sink;
    };
}