#pragma once

#include "logpp/core/LogLevel.h"
#include "logpp/sinks/Sink.h"

namespace logpp
{
    struct StringLiteral
    {
        explicit StringLiteral(const char* value)
            : value(value)
        {}

        const char* value;
    };

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

    template<typename KeyOffset, typename OffsetT>
    struct StructuredOffset
    {
        KeyOffset key;
        OffsetT value;
    };

    template<typename KeyOffset, typename OffsetT>
    StructuredOffset<KeyOffset, OffsetT> structuredOffset(KeyOffset key, OffsetT value)
    {
        return { key, value };
    }

    namespace details
    {
        template< typename T >
        using OffsetT = decltype(static_cast< LogBufferBase* >(nullptr)->write(std::declval< T >()));

        template<typename KeyStr>
        auto writeKey(LogBufferBase& buffer, const KeyStr& key)
        {
            return buffer.write(key);
        }

        auto writeKey(LogBufferBase& buffer, StringLiteral key)
        {
            return StringLiteralOffset { key.value };
        }

        template<typename Arg>
        auto write(LogBufferBase& buffer, const Arg& arg)
        {
            auto keyOffset = writeKey(buffer, arg.key);
            auto valueOffset = buffer.write(arg.value);

            return structuredOffset(keyOffset, valueOffset);
        }

        template< typename... Args >
        auto writeAll(LogBufferBase& buffer, Args&& ...args)
        {
            return std::make_tuple(write(buffer, args)...);
        }

        template<typename> struct StructuredEvent;

        #pragma pack(push, 1)
        template<typename... Args>
        struct StructuredEvent<std::tuple<Args...>>
        {
            using Offsets = std::tuple<Args...>;

            StructuredEvent(Offsets offsets)
                : offsets{offsets}
            {}

            void format(LogBufferView view, LogWriter& writer) const
            {
                formatImpl(view, writer, std::index_sequence_for<Args...>());
            }

        private:
            Offsets offsets;

            template<size_t... Indexes>
            void formatImpl(LogBufferView view, LogWriter& writer, std::index_sequence<Indexes...>) const
            {
                auto doFormat = [&](const auto& structuredOffset)
                {
                    auto key = structuredOffset.key.get(view);
                    writer.write(key, view, structuredOffset.value);
                };

                (doFormat(std::get<Indexes>(offsets)), ...);
            }
        };
        #pragma pack(pop)

        template<typename Offsets>
        StructuredEvent<Offsets> structuredEvent(Offsets offsets)
        {
            return { offsets };
        }
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
        void log(Str text, LogLevel level, Args&&... args)
        {
            EventLogBuffer buffer;

            auto textOffset = buffer.write(text);
            auto offsets = details::writeAll(buffer, std::forward<Args>(args)...);

            auto event = details::structuredEvent(offsets);
            buffer.writeEvent(event);

            m_sink->format(name(), level, buffer, textOffset);
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

    private:
        std::string m_name;
        LogLevel m_level;

        std::shared_ptr<sink::Sink> m_sink;
    };
}