#include "logpp/core/LoggerRegistry.h"

#include "logpp/sinks/ColoredConsole.h"
#include "logpp/sinks/file/FileSink.h"
#include "logpp/sinks/file/RollingFileSink.h"

#include <iostream>

namespace logpp
{
    LoggerRegistry::LoggerRegistry()
    {
        auto sink = std::make_shared<sink::ColoredOutputConsole>();
        setDefaultLogger(std::make_shared<Logger>("logpp", LogLevel::Debug, sink));

        registerSink<sink::ColoredOutputConsole>();
        registerSink<sink::ColoredErrorConsole>();
        registerSink<sink::FileSink>();
        registerSink<sink::RollingFileSink>();
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

    bool LoggerRegistry::registerLogger(std::shared_ptr<Logger> logger)
    {
        return registerLoggerFunc(std::string(logger->name()), [=](std::string_view) {
            return logger;
        });
    }

    bool LoggerRegistry::registerLoggerFunc(std::string name, LoggerRegistry::LoggerFactory factory)
    {
        auto it = m_loggerFactories.insert(std::make_pair(std::move(name), std::move(factory)));
        return it.second;
    }

    std::shared_ptr<Logger> LoggerRegistry::get(std::string_view name)
    {
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

        return m_defaultLogger;
    }

    std::shared_ptr<Logger> LoggerRegistry::defaultLogger()
    {
        return m_defaultLogger;
    }

    void LoggerRegistry::setDefaultLogger(std::shared_ptr<Logger> logger)
    {
        m_defaultLogger = std::move(logger);
    }


    std::shared_ptr<sink::Sink> LoggerRegistry::createSink(std::string_view name)
    {
        auto sinkIt = m_sinks.find(name);
        if (sinkIt == std::end(m_sinks))
        {
            auto factoryIt = m_sinkFactories.find(name);
            if (factoryIt == std::end(m_sinkFactories))
                return nullptr;

            auto factory = factoryIt->second;
            sinkIt = m_sinks.insert(std::make_pair(std::string(name), factory())).first;
        }

        return sinkIt->second;
    }

    std::shared_ptr<sink::Sink> LoggerRegistry::findSink(std::string_view name) const
    {
        auto it = m_sinks.find(name);
        if (it == std::end(m_sinks))
            return nullptr;

        return it->second;
    }
}