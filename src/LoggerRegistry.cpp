#include "logpp/core/LoggerRegistry.h"
#include "logpp/sinks/ColoredConsole.h"

#include <iostream>

namespace logpp
{
    LoggerRegistry::LoggerRegistry()
    {
        auto sink = std::make_shared<sink::ColoredOutputConsole>();
        setDefaultLogger(std::make_shared<Logger>("logpp", LogLevel::Debug, sink));

        registerSink<sink::ColoredOutputConsole>();
        registerSink<sink::ColoredErrorConsole>();
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
            auto factoryIt = m_loggerFactories.find(fragment);
            if (factoryIt != std::end(m_loggerFactories))
                return std::invoke(factoryIt->second, std::string(name));
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
}