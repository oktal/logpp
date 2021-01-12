#include "logpp/config/TomlConfigurator.h"

#include "logpp/core/Logger.h"
#include "logpp/core/LogLevel.h"

namespace logpp
{
    std::optional<TomlConfigurator::Error> TomlConfigurator::configure(std::string_view config)
    {
        return configure(config, LoggerRegistry::defaultRegistry());
    }

    std::optional<TomlConfigurator::Error> TomlConfigurator::configure(std::string_view config, LoggerRegistry& registry)
    {
        try
        {
            auto table = toml::parse(config);
            auto [sinks, err] = parseSinks(table);
            if (err)
                return err;

            err = parseLoggers(table, sinks, registry);
            if (err)
                return err;

        }
        catch (const toml::parse_error& e)
        {
            return Error::from(e);
        }
        
        return std::nullopt;
    }

    std::pair<std::vector<TomlConfigurator::Sink>, std::optional<TomlConfigurator::Error>> TomlConfigurator::parseSinks(const toml::table& table)
    {
        auto sinksNode = table["sinks"];

        auto* sinksTable = sinksNode.as_table();
        if (!sinksTable)
            return std::make_pair(std::vector<Sink>{}, Error { "sinks: expected table", table.source()});

        std::vector<Sink> sinks;

        for (auto&& [name, sinkNode]: *sinksTable)
        {
            if (auto* sinkTable = sinkNode.as_table())
            {
                auto [sink, err] = parseSink(name, *sinkTable);
                if (err)
                    return std::make_pair(std::vector<Sink>{}, err);
                sinks.push_back(std::move(*sink));
            }
        }

        return std::make_pair(std::move(sinks), std::nullopt);
    }

    std::pair<std::optional<TomlConfigurator::Sink>, std::optional<TomlConfigurator::Error>> TomlConfigurator::parseSink(std::string name, const toml::table& table)
    {
        auto [type, err] = tryRead<std::string>(table, "type", "sink.type: expected string");
        if (err)
            return std::make_pair(std::nullopt, err);

        Sink sink;
        sink.name = std::move(name);
        sink.type = *type;

        if (auto* options = table["options"].as_table())
        {
            for (auto&& [key, valueNode]: *options)
            {
                std::string valueString;
                valueNode.visit([&](const auto& node) {
                    if constexpr (toml::is_integer<decltype(node)>)
                        valueString = std::to_string(node.get());
                    else if constexpr (toml::is_string<decltype(node)>)
                        valueString = node.get();
                });
                auto ok = sink.options.insert(std::make_pair(key, std::move(valueString))).second;
                if (!ok)
                    return std::make_pair(std::nullopt, Error { "duplicated option", valueNode.source() });
            }
        }

        return std::make_pair(std::move(sink), std::nullopt);
    }

    std::optional<TomlConfigurator::Error> TomlConfigurator::parseLoggers(const toml::table& table, const std::vector<Sink>& sinks, LoggerRegistry& registry)
    {
        auto loggersTable = table["loggers"].as_table();
        if (!loggersTable)
            return Error{ "expected loggers", table.source() };

        for (auto&& [name, loggerNode]: *loggersTable)
        {
            auto* loggerTable = loggerNode.as_table();
            if (!loggerTable)
                return Error{ "expected logger", loggerNode.source()};

            auto err = parseLogger(name, *loggerTable, sinks, registry);
            if (err)
                return err;
        }

        return std::nullopt;
    }

    std::optional<TomlConfigurator::Error> TomlConfigurator::parseLogger(std::string, const toml::table& table, const std::vector<Sink>& sinks, LoggerRegistry& registry)
    {
        auto [name, err1] = tryRead<std::string>(table, "name", "logger.name: expected string");
        if (err1)
            return err1;

        auto [levelStr, err2] = tryRead<std::string>(table, "level", "logger.level: expected string");
        if (err2)
            return err2;

        auto level = parseLevel(*levelStr);
        if (!level)
            return Error { "logger: unknown level", table["level"].as_string()->source() };

        auto sinksNode = table["sinks"];
        auto sinksArray = sinksNode.as_array();
        if (!sinksArray)
            return Error { "logger: expected sinks", table.source() };

        for (const auto& sinkNode: *sinksArray)
        {
            auto sinkName = sinkNode.value<std::string>();
            if (!sinkName)
                return Error { "sink: expected string", sinkNode.source() };

            auto it = std::find_if(std::begin(sinks), std::end(sinks), [&](const auto& sink) {
                return sink.name == *sinkName;
            });

            if (it == std::end(sinks))
                return Error { "logger: unknown sink reference", sinkNode.source() };

            const auto& sinkRef = *it;
            auto sink = registry.createSink(sinkRef.type);
            if (!sink)
                return Error { "logger: unknown sink type", sinkNode.source() };

            for (const auto& option: sinkRef.options) 
            {
                if (!sink->setOption(option.first, option.second))
                    return Error { "logger: unknown sink option", sinkNode.source() };
            }

            auto res = registry.registerLoggerFunc(*name, [=](std::string name) {
                return std::make_shared<Logger>(std::move(name), *level, sink);
            });

            if (!res)
                return Error { "logger: failed to register logger", table.source() };
        }

        return std::nullopt;
    }
}