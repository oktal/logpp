#include "logpp/config/TomlConfigurator.h"

#include "logpp/core/Logger.h"
#include "logpp/core/LogLevel.h"

#include "logpp/sinks/Sink.h"

#include <fstream>

namespace logpp
{
    namespace
    {
        template<typename Node>
        std::optional<std::string> toString(const Node& node)
        {
            if (auto* strVal = node.as_string())
                return strVal->get();
            else if (auto* intVal = node.as_integer())
                return std::to_string(intVal->get());
            else if (auto* floatVal = node.as_floating_point())
                return std::to_string(floatVal->get());
            else if (auto* boolVal = node.as_boolean())
                return boolVal->get() ? "true" : "false";

            return std::nullopt;
        }

        template<typename Node>
        std::optional<TomlConfigurator::Error> addSinkOption(sink::Options& options, std::string key, const Node& node)
        {
            if constexpr (toml::is_integer<Node>)
            {
                if (!options.add(std::move(key), std::to_string(node.get())))
                    return TomlConfigurator::Error { "invalid option", node.source() };

                return std::nullopt;
            }
            else if constexpr (toml::is_string<Node>)
            {
                if (!options.add(std::move(key), node.get()))
                    return TomlConfigurator::Error { "invalid option", node.source() };

                return std::nullopt;
            }
            else if constexpr (toml::is_array<Node>)
            {
                sink::Options::Array arr;
                for (const auto& valueNode: node)
                {
                    auto str = toString(valueNode);
                    if (!str)
                        return TomlConfigurator::Error { "invalid array value type", valueNode.source() };

                    arr.push_back(std::move(*str));
                }

                if (!options.add(std::move(key), std::move(arr)))
                    return TomlConfigurator::Error { "could not add option", node.source() };

                return std::nullopt;
            }
            else if constexpr (toml::is_table<Node>)
            {
                sink::Options::Dict dict;
                for (auto&& [key, valueNode]: node)
                {
                    auto str = toString(valueNode);
                    if (!str)
                        return TomlConfigurator::Error { "invalid table value type", valueNode.source() };

                    auto res = dict.insert(std::make_pair(std::move(key), std::move(*str))).second;
                    if (!res)
                        return TomlConfigurator::Error { "could not add table value", valueNode.source() };
                }

                if (!options.add(std::move(key), std::move(dict)))
                    return TomlConfigurator::Error { "could not add option", node.source() };

                return std::nullopt;
            }

            return TomlConfigurator::Error { "invalid option type", node.source() };
        }
    }

    std::optional<TomlConfigurator::Error> TomlConfigurator::configure(std::string_view config)
    {
        return configure(config, LoggerRegistry::defaultRegistry());
    }

    std::optional<TomlConfigurator::Error> TomlConfigurator::configure(std::string_view config, LoggerRegistry& registry)
    {
        return configure([&] { return toml::parse(config); }, registry);
    }

    std::optional<TomlConfigurator::Error> TomlConfigurator::configureFile(std::string_view path)
    {
        return configureFile(path, LoggerRegistry::defaultRegistry());
    }

    std::optional<TomlConfigurator::Error> TomlConfigurator::configureFile(std::string_view path, LoggerRegistry& registry)
    {
        return configure([&] { return toml::parse_file(path); }, registry);
    }

    std::pair<std::vector<TomlConfigurator::Sink>, std::optional<TomlConfigurator::Error>> TomlConfigurator::parseSinks(const toml::table& table)
    {
        auto sinksNode = table["sinks"];
        if (!sinksNode)
            return std::make_pair(std::vector<Sink>{}, std::nullopt);

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
                auto&& k = key;
                auto err = valueNode.visit([&sink, k](const auto& node) {
                    return addSinkOption(sink.options, k, node);
                });
                if (err)
                    return std::make_pair(std::nullopt, err);
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

            if (!sink->activateOptions(sinkRef.options))
                return Error { "sink: failed to activate options", sinkNode.source() };

            auto res = registry.registerLoggerFunc(*name, [=](std::string name) {
                return std::make_shared<Logger>(std::move(name), *level, sink);
            });

            if (!res)
                return Error { "logger: failed to register logger", table.source() };
        }

        return std::nullopt;
    }
}