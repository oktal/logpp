#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/core/LogLevel.h"
#include "logpp/core/Offset.h"

#include "logpp/utils/detect.h"

#include <fmt/format.h>

#include <unordered_map>
#include <variant>
#include <vector>

namespace logpp::sink
{
    class Options
    {
    public:
        using Array = std::vector<std::string>;
        using Dict  = std::unordered_map<std::string, std::string>;

        struct Value
        {
            using Type = std::variant<std::string, Array, Dict>;

            template <typename T>
            Value(T&& val)
                : m_val(std::forward<T>(val))
            { }

            template <typename T>
            std::optional<T> as() const
            {
                auto* val = std::get_if<T>(&m_val);
                if (val)
                    return *val;

                return std::nullopt;
            }

            std::optional<std::string> asString() const
            {
                return as<std::string>();
            }

            std::optional<Array> asArray() const
            {
                return as<Array>();
            }

            std::optional<Dict> asDict() const
            {
                return as<Dict>();
            }

        private:
            Type m_val;
        };

        using Key           = std::string;
        using Values        = std::unordered_map<Key, Value>;
        using Iterator      = Values::iterator;
        using ConstIterator = Values::const_iterator;

        bool add(std::string key, Value value)
        {
            return m_values.insert(std::make_pair(std::move(key), std::move(value))).second;
        }

        std::optional<Value> tryGet(const Key& key) const
        {
            auto it = m_values.find(key);
            if (it == std::end(m_values))
                return std::nullopt;

            return it->second;
        }

        Iterator begin()
        {
            return m_values.begin();
        }

        Iterator end()
        {
            return m_values.end();
        }

        ConstIterator begin() const
        {
            return m_values.begin();
        }

        ConstIterator end() const
        {
            return m_values.end();
        }

    private:
        Values m_values;
    };

    class ConfigurationError : public std::runtime_error
    {
    public:
        explicit ConfigurationError(const char* reason)
            : runtime_error(reason)
        { }

        explicit ConfigurationError(std::string reason)
            : runtime_error(std::move(reason))
        { }

        explicit ConfigurationError(std::string_view reason)
            : runtime_error(std::string(reason))
        { }
    };

    class Sink
    {
    public:
        virtual ~Sink() = default;

        virtual bool activateOptions(const Options& options)                                   = 0;
        virtual void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) = 0;
    };

    class SinkBase : public Sink
    {
    protected:
        static void raiseConfigurationError(std::string_view reason)
        {
            throw ConfigurationError(reason);
        }

        template <typename... Args>
        static void raiseConfigurationError(const char* formatStr, Args&&... args)
        {
            throw ConfigurationError(fmt::format(formatStr, std::forward<Args>(args)...));
        }
    };

    namespace details
    {
        template <typename Sink>
        using Name = decltype(&Sink::Name);

        template <typename Sink>
        constexpr bool HasName = logpp::is_detected_v<Name, Sink>;
    }

    namespace concepts
    {
        template <typename T>
        using Sink = std::integral_constant<
            bool,
            std::is_base_of_v<Sink, T> && details::HasName<T>>;

        template <typename T>
        constexpr bool IsSink = Sink<T>::value;
    }

}
