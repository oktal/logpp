#include "logpp/core/LoggerRegistry.h"

#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/ColoredConsole.h"
#include "logpp/sinks/Console.h"
#include "logpp/sinks/file/FileSink.h"
#include "logpp/sinks/file/RollingFileSink.h"

#include <iostream>

namespace logpp
{
    class NoopSink : public sink::Sink
    {
    public:
        void activateOptions(const sink::Options&) override
        { }

        void sink(std::string_view, LogLevel, const EventLogBuffer&) override
        { }
    };

    std::shared_ptr<Logger> LoggerRegistry::LoggerInstantiator::createInstance(const std::string& name)
    {
        return std::make_shared<Logger>(name, LogLevel::Off, std::make_shared<NoopSink>());
    }

    LoggerRegistry::LoggerRegistry()
    {
        auto defaultSink = std::make_shared<sink::ColoredOutputConsole>();
        setDefaultLogger(std::make_shared<Logger>("logpp", LogLevel::Debug, defaultSink));

        registerSinkFactory<sink::AsyncSink>();
        registerSinkFactory<sink::ColoredOutputConsole>();
        registerSinkFactory<sink::ColoredErrorConsole>();
        registerSinkFactory<sink::OutputConsole>();
        registerSinkFactory<sink::ErrorConsole>();
        registerSinkFactory<sink::FileSink>();
        registerSinkFactory<sink::RollingFileSink>();

        m_defaultLoggerFactory = [&](std::string name) {
            return m_loggerInstantiator.create(name);
        };
    }

    bool LoggerRegistry::matches(const LoggerKey& key, std::string_view name)
    {
        // return std::find(key.rbegin(), key.rend(), name) != key.rend();
        for (auto fragmentIt = key.rbegin(); fragmentIt != key.rend(); ++fragmentIt)
        {
            if (*fragmentIt == name)
                return true;
        }
        return false;
    }

    LoggerRegistry& LoggerRegistry::defaultRegistry()
    {
        static LoggerRegistry instance;
        return instance;
    }

    bool LoggerRegistry::registerLogger(std::shared_ptr<Logger> logger, bool isDefault)
    {
        return registerLoggerFunc(
            std::string(logger->name()), [=](std::string_view) {
                return logger;
            },
            isDefault);
    }

    bool LoggerRegistry::registerLoggerFunc(std::string name, LoggerRegistry::LoggerFactory factory, bool isDefault)
    {
        std::lock_guard guard(m_mutex);

        if (isDefault)
        {
            if (m_loggerInstantiator.totalInstances() > 0)
            {
                auto tmpLogger = factory("");
                m_loggerInstantiator.forEachInstance(
                    [&](const std::shared_ptr<Logger>& instance) {
                        instance->setSink(tmpLogger->sink());
                        instance->setLevel(tmpLogger->level());
                    });
            }
            m_defaultLoggerFactory = factory;
        }

        auto it = m_loggerFactories.insert(std::make_pair(std::move(name), std::move(factory)));
        return it.second;
    }

    std::shared_ptr<Logger> LoggerRegistry::get(std::string_view name)
    {
        std::lock_guard guard(m_mutex);

        LoggerKey key(name);

        for (auto fragmentIt = key.rbegin(); fragmentIt != key.rend(); ++fragmentIt)
        {
            auto fragment = *fragmentIt;

            auto loggerIt = m_loggers.find(fragment);
            if (loggerIt != std::end(m_loggers))
                return loggerIt->second;

            auto factoryIt = m_loggerFactories.find(fragment);
            if (factoryIt != std::end(m_loggerFactories))
            {
                auto logger = std::invoke(factoryIt->second, std::string(name));
                m_loggers.insert(std::make_pair(std::string(name), logger));
                return logger;
            }
        }

        if (m_defaultLoggerFactory)
        {
            auto logger = std::invoke(m_defaultLoggerFactory, std::string(name));
            m_loggers.insert(std::make_pair(std::string(name), logger));
            return logger;
        }

        return m_defaultLogger;
    }

    std::shared_ptr<Logger> LoggerRegistry::defaultLogger()
    {
        std::lock_guard guard(m_mutex);
        return m_defaultLogger;
    }

    void LoggerRegistry::setDefaultLogger(std::shared_ptr<Logger> logger)
    {
        registerLogger(logger, true);
    }

    void LoggerRegistry::setDefaultLoggerFunc(LoggerFactory factory)
    {
        std::lock_guard guard(m_mutex);
        if (m_loggerInstantiator.totalInstances() > 0)
        {
            auto tmpLogger = factory("");
            m_loggerInstantiator.forEachInstance(
                [&](const std::shared_ptr<Logger>& instance) {
                    instance->setSink(tmpLogger->sink());
                    instance->setLevel(tmpLogger->level());
                });
        }
        m_defaultLoggerFactory = factory;
    }

    std::shared_ptr<sink::Sink> LoggerRegistry::createSink(std::string_view name)
    {
        std::lock_guard guard(m_mutex);

        auto factoryIt = m_sinkFactories.find(name);
        if (factoryIt == std::end(m_sinkFactories))
            return nullptr;

        auto factory = factoryIt->second;
        return factory();
    }

    bool LoggerRegistry::registerSink(std::string name, std::shared_ptr<sink::Sink> sink)
    {
        std::lock_guard guard(m_mutex);

        return m_sinks.insert(std::make_pair(std::move(name), std::move(sink))).second;
    }

    std::shared_ptr<sink::Sink> LoggerRegistry::findSink(std::string_view name) const
    {
        std::lock_guard guard(m_mutex);

        auto it = m_sinks.find(name);
        if (it == std::end(m_sinks))
            return nullptr;

        return it->second;
    }
}
