#include "logpp/core/LoggerRegistry.h"

#include "logpp/sinks/ColoredOutputConsole.h"

namespace logpp
{
    LoggerRegistry::LoggerRegistry()
    {
        auto sink = std::make_shared<sink::ColoredOutputConsole>();
        setDefaultLogger(std::make_shared<Logger>("logpp", LogLevel::Debug, sink));
    }

    LoggerRegistry& LoggerRegistry::defaultRegistry()
    {
        static LoggerRegistry instance;
        return instance;
    }

    bool LoggerRegistry::registerLogger(std::shared_ptr<Logger> logger)
    {
        return m_loggers.insert(std::make_pair(logger->name(), std::move(logger))).second;
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