#pragma once

#include "logpp/core/Logger.h"
#include "logpp/sinks/Sink.h"
#include "logpp/format/Formatter.h"

#include <map>
#include <iostream>

namespace logpp
{
    struct LoggerKey
    {
        template<typename Strategy>
        struct BasicIterator
        {
            explicit BasicIterator(
                std::string_view key,
                std::string_view fragment,
                std::string_view::size_type pos
            )
                : key(key)
                , fragment(fragment)
                , pos(pos)
            {}

            std::string_view operator*() const
            {
                return fragment;
            }

            BasicIterator& operator++()
            {
                std::tie(fragment, pos) = Strategy::next(key, pos);
                return *this;
            }

            BasicIterator operator++(int)
            {
                BasicIterator tmp(*this);
                ++tmp;
                return tmp;
            }

            bool operator==(BasicIterator other) const
            {
                return key == other.key &&
                       fragment == other.fragment &&
                       pos == other.pos;
            }

            bool operator!=(BasicIterator other) const
            {
                return !(*this == other);
            }

            std::string_view key;
            std::string_view fragment;
            std::string_view::size_type pos;
        };

        struct ForwardStrategy
        {
            static std::pair<std::string_view, std::string_view::size_type> next(std::string_view key, std::string_view::size_type pos)
            {
                if (pos == key.size())
                    return { std::string_view{}, std::string_view::npos };

                auto separatorPos = key.find('.', pos);
                if (separatorPos == std::string_view::npos)
                    return { key, key.size() };
                else
                    return { key.substr(0, separatorPos), separatorPos + 1};
            }
        };

        struct ReverseStrategy
        {
            static std::pair<std::string_view, std::string_view::size_type> next(std::string_view key, std::string_view::size_type pos)
            {
                auto separatorPos = key.rfind('.', pos);
                if (separatorPos == std::string_view::npos)
                    return { std::string_view{}, std::string_view::npos };
                else
                    return { key.substr(0, separatorPos), separatorPos - 1};
            }
        };

        using Iterator = BasicIterator<ForwardStrategy>;
        using ReverseIterator = BasicIterator<ReverseStrategy>;

        explicit LoggerKey(std::string_view value)
            : m_value(value)
        {}

        Iterator begin() const
        {
            std::string_view fragment;
            auto separatorPos = m_value.find('.');
            if (separatorPos == std::string_view::npos)
                fragment = m_value;
            else
                fragment = m_value.substr(0, separatorPos);

            return Iterator { m_value, fragment, separatorPos + 1 };
        }

        Iterator end() const
        {
            return Iterator { m_value,  std::string_view{}, std::string_view::npos };
        }

        ReverseIterator rbegin() const
        {
            return ReverseIterator { m_value, m_value, std::string_view::npos };
        }

        ReverseIterator rend() const
        {
            return ReverseIterator { m_value, std::string_view{}, std::string_view::npos };
        }

        std::string_view value() const
        {
            return m_value;
        }

    private:
        std::string_view m_value;
    };

    class LoggerRegistry
    {
    public:
        using SinkFactory = std::function<std::shared_ptr<sink::Sink>()>;
        using LoggerFactory = std::function<std::shared_ptr<Logger>(std::string)>;

        LoggerRegistry();

        static bool matches(const LoggerKey& key, std::string_view name);

        bool registerLogger(std::shared_ptr<Logger> logger);
        bool registerLoggerFunc(std::string name, LoggerFactory factory);

        std::shared_ptr<Logger> get(std::string_view name);

        template<typename LoggerFunc>
        void forEachLogger(LoggerFunc&& loggerFunc) const
        {
            for (const auto& [name, logger]: m_loggers)
            {
                std::invoke(loggerFunc, name, logger);
            }
        }

        std::shared_ptr<Logger> defaultLogger();
        void setDefaultLogger(std::shared_ptr<Logger> logger);

        template<typename Sink>
        bool registerSink()
        {
            static_assert(sink::concepts::IsSink<Sink>, "Sink must be satisfy the Sink concept");

            auto factory = [] { return std::make_shared<Sink>(); };
            auto it = m_sinkFactories.insert(std::make_pair(std::string(Sink::Name), std::move(factory)));
            return it.second;
        }

        template<typename Sink>
        bool hasSink()
        {
            static_assert(sink::concepts::IsSink<Sink>, "Sink must be satisfy the Sink concept");

            return hasSink(Sink::Name);
        }

        bool hasSink(std::string_view type) const;

        std::shared_ptr<sink::Sink> createSink(std::string_view name);
        std::shared_ptr<sink::Sink> findSink(std::string_view name) const;

        template<typename Sink, typename SinkFunc>
        void forEachSinkOfType(SinkFunc&& func) const
        {
            static_assert(sink::concepts::IsSink<Sink>, "Sink must be satisfy the Sink concept");

            for (const auto& [name, sink]: m_sinks)
            {
                if (auto s = std::dynamic_pointer_cast<Sink>(sink))
                    std::invoke(func, name, s);
            }
        }

        template<typename SinkFunc>
        void forEachSink(SinkFunc&& func)
        {
            for (auto& [name, sink]: m_sinks)
            {
                std::invoke(func, name, sink);
            }
        }

        template<typename SinkFunc>
        void forEachSink(SinkFunc&& func) const
        {
            for (const auto& [name, sink]: m_sinks)
            {
                std::invoke(func, name, sink);
            }
        }

        static LoggerRegistry& defaultRegistry();
    private:
        std::shared_ptr<Logger> m_defaultLogger;

        // note: We are explicitely using std::less<> as a comparator
        // to benefit from the c++14 is_transparent heterogenous feature
        // and be able to find with string_view as a key type
        std::map<std::string, LoggerFactory, std::less<>> m_loggerFactories;
        std::map<std::string, SinkFactory, std::less<>> m_sinkFactories;

        std::map<std::string, std::shared_ptr<Logger>, std::less<>> m_loggers;
        std::map<std::string, std::shared_ptr<sink::Sink>, std::less<>> m_sinks;
    };
}