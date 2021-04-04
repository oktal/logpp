#include "logpp/config/TomlConfigurator.h"

#include "logpp/core/LogLevel.h"
#include "logpp/core/Logger.h"

#include "logpp/sinks/LevelSink.h"
#include "logpp/sinks/MultiSink.h"
#include "logpp/sinks/Sink.h"

#include <fstream>
#include <iostream>

namespace logpp
{
    namespace
    {
        template <typename Node>
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

        template <typename Node>
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
                for (const auto& valueNode : node)
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
                for (auto&& [key, valueNode] : node)
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

    std::pair<std::optional<FileWatcher::WatchId>, std::optional<TomlConfigurator::Error>>
    TomlConfigurator::configureFileAndWatch(std::string_view path, std::shared_ptr<FileWatcher> watcher)
    {
        return configureFileAndWatch(path, std::move(watcher), LoggerRegistry::defaultRegistry());
    }

    std::pair<std::optional<FileWatcher::WatchId>, std::optional<TomlConfigurator::Error>>
    TomlConfigurator::configureFileAndWatch(std::string_view path, std::shared_ptr<FileWatcher> watcher, LoggerRegistry& registry)
    {
        auto err = configureFile(path, registry);
        if (err)
            return std::make_pair(std::nullopt, std::move(err));

        auto watchId = watcher->addWatch(path, [=, &registry](std::string_view path) {
            try
            {
                auto table  = toml::parse_file(path);
                auto result = parseConfiguration(table);
                if (result.error)
                {
                    std::cerr << "[logpp] Error reading file " << path << ": " << *result.error << std::endl;
                    return;
                }

                for (const auto& loggerConfig : result.loggers)
                {
                    registry.forEachLogger([&](const std::string& name, const std::shared_ptr<logpp::Logger>& logger) {
                        LoggerKey key(loggerConfig.name);
                        if (LoggerRegistry::matches(key, name))
                        {
                            if (logger->level() != loggerConfig.level)
                                logger->setLevel(loggerConfig.level);
                        }
                    });
                }

                std::cout << "[logpp] Configuration " << path << " has been reloaded\n";
            }
            catch (const std::exception& e)
            {
                std::cerr << "[logpp] Error reading file " << path << ": " << e.what() << '\n';
            }
        });

        if (!watchId)
            return std::make_pair(std::nullopt, Error { "Failed to add watch", std::nullopt });

        return std::make_pair(watchId, std::nullopt);
    }

    TomlConfigurator::ParseResult
    TomlConfigurator::parseConfiguration(const toml::table& table)
    {
        auto [sinks, err1] = parseSinks(table);
        if (err1)
            return ParseResult::failure(*err1);

        auto [loggers, err2] = parseLoggers(table, sinks);
        if (err2)
            return ParseResult::failure(*err2);

        return ParseResult::success(std::move(sinks), std::move(loggers));
    }

    std::pair<std::vector<TomlConfigurator::Sink>, std::optional<TomlConfigurator::Error>>
    TomlConfigurator::parseSinks(const toml::table& table)
    {
        auto sinksNode = table["sinks"];
        if (!sinksNode)
            return std::make_pair(std::vector<Sink> {}, std::nullopt);

        auto* sinksTable = sinksNode.as_table();
        if (!sinksTable)
            return std::make_pair(std::vector<Sink> {}, Error { "sinks: expected table", table.source() });

        std::vector<Sink> sinks;

        for (auto&& [name, sinkNode] : *sinksTable)
        {
            if (auto* sinkTable = sinkNode.as_table())
            {
                auto [sink, err] = parseSink(name, *sinkTable);
                if (err)
                    return std::make_pair(std::vector<Sink> {}, err);
                sinks.push_back(std::move(*sink));
            }
        }

        return std::make_pair(std::move(sinks), std::nullopt);
    }

    std::pair<std::optional<TomlConfigurator::Sink>, std::optional<TomlConfigurator::Error>>
    TomlConfigurator::parseSink(std::string name, const toml::table& table)
    {
        auto [type, err] = tryRead<std::string>(table, "type", "sink.type: expected string");
        if (err)
            return std::make_pair(std::nullopt, err);

        std::optional<LogLevel> level;
        if (table.contains("level"))
        {
            auto levelString = table["level"].value<std::string>();
            if (!levelString)
                return std::make_pair(std::nullopt, Error { "sink.level: expected string", table.source() });

            auto levelValue = parseLevel(*levelString);
            if (!levelValue)
                return std::make_pair(std::nullopt, Error { "sink.level: unknown level", table.source() });

            level = levelValue;
        }

        sink::Options sinkOptions;
        if (auto* options = table["options"].as_table())
        {
            for (auto&& [key, valueNode] : *options)
            {
                auto&& k = key;
                auto err = valueNode.visit([&sinkOptions, k](const auto& node) {
                    return addSinkOption(sinkOptions, k, node);
                });
                if (err)
                    return std::make_pair(std::nullopt, err);
            }
        }

        return std::make_pair(Sink { std::move(name), *type, level, std::move(sinkOptions), table.source() }, std::nullopt);
    }

    std::pair<std::vector<TomlConfigurator::Logger>, std::optional<TomlConfigurator::Error>>
    TomlConfigurator::parseLoggers(const toml::table& table, const std::vector<Sink>& sinks)
    {
        static auto error = [](Error err) { return std::make_pair(std::vector<Logger> {}, std::move(err)); };

        auto loggersTable = table["loggers"].as_table();
        if (!loggersTable)
            return error(Error { "expected loggers", table.source() });

        std::vector<Logger> loggers;
        for (auto&& [name, loggerNode] : *loggersTable)
        {
            auto* loggerTable = loggerNode.as_table();
            if (!loggerTable)
                return error(Error { "expected logger", loggerNode.source() });

            auto [logger, err] = parseLogger(name, *loggerTable, sinks);
            if (err)
                return error(*err);

            loggers.push_back(*logger);
        }

        return std::make_pair(std::move(loggers), std::nullopt);
    }

    std::pair<std::optional<TomlConfigurator::Logger>, std::optional<TomlConfigurator::Error>>
    TomlConfigurator::parseLogger(std::string, const toml::table& table, const std::vector<Sink>& sinks)
    {
        static auto error = [](Error err) { return std::make_pair(std::nullopt, std::move(err)); };

        auto [name, err1] = tryRead<std::string>(table, "name", "logger.name: expected string");
        if (err1)
            return error(*err1);

        auto [levelStr, err2] = tryRead<std::string>(table, "level", "logger.level: expected string");
        if (err2)
            return error(*err2);

        auto level = parseLevel(*levelStr);
        if (!level)
            return error(Error { "logger: unknown level", table["level"].as_string()->source() });

        auto [isDefault, err3] = tryReadOr<bool>(table, "default", "logger.default: expected bool", false);
        if (err3)
            return error(*err3);

        auto sinksNode  = table["sinks"];
        auto sinksArray = sinksNode.as_array();
        if (!sinksArray)
            return error(Error { "logger: expected sinks", table.source() });

        std::vector<Sink> loggerSinks;
        for (const auto& sinkNode : *sinksArray)
        {
            auto sinkName = sinkNode.value<std::string>();
            if (!sinkName)
                return error(Error { "sink: expected string", sinkNode.source() });

            auto it = std::find_if(std::begin(sinks), std::end(sinks), [&](const auto& sink) {
                return sink.name == *sinkName;
            });

            if (it == std::end(sinks))
                return error(Error { "logger: unknown sink reference", sinkNode.source() });

            const auto& sink = *it;
            loggerSinks.push_back(sink);
        }

        return std::make_pair(Logger { *name, *level, std::move(loggerSinks), *isDefault, table.source() }, std::nullopt);
    }

    std::optional<TomlConfigurator::Error>
    TomlConfigurator::configureSinks(LoggerRegistry& registry, std::vector<TomlConfigurator::Sink> sinks)
    {
        // We need to register sinks in the same order they appear in the configuration file
        // because some sinks like `AsyncSink` depend on sinks being previously registered.
        std::sort(std::begin(sinks), std::end(sinks), [](const Sink& lhs, const Sink& rhs) {
            return lhs.sourceRegion.begin.line < rhs.sourceRegion.begin.line;
        });

        for (const auto& sinkConfig : sinks)
        {
            auto sink = registry.createSink(sinkConfig.type);
            if (!sink)
                return Error { "sink: unknown type", sinkConfig.sourceRegion };

            if (sinkConfig.level)
                sink = std::make_shared<sink::LevelSink>(std::move(sink), *sinkConfig.level);

            try
            {
                sink->activateOptions(sinkConfig.options);
            }
            catch (const sink::ConfigurationError& e)
            {
                auto reason = fmt::format("sink: invalid configuration: {}", e.what());
                return Error { std::move(reason), sinkConfig.sourceRegion };
            }

            if (!registry.registerSink(std::string(sinkConfig.name), std::move(sink)))
                return Error { "sink: sink already exists", sinkConfig.sourceRegion };
        }

        return std::nullopt;
    }

    std::optional<TomlConfigurator::Error>
    TomlConfigurator::configureLoggers(LoggerRegistry& registry, const std::vector<TomlConfigurator::Logger>& loggers)
    {

        for (const auto& logger : loggers)
        {
            std::vector<std::shared_ptr<sink::Sink>> sinks;
            for (const auto& sinkConfig : logger.sinks)
            {
                auto sink = registry.findSink(sinkConfig.name);
                if (!sink)
                    return Error { "logger: unknown sink", sinkConfig.sourceRegion };

                sinks.push_back(std::move(sink));
            }

            auto sink = sinks.size() > 1 ? std::make_shared<sink::MultiSink>(sinks) : sinks[0];
            auto res  = registry.registerLoggerFunc(logger.name, [=](std::string name) {
                return std::make_shared<logpp::Logger>(std::move(name), logger.level, sink);
            }, logger.isDefault);

            if (!res)
                return Error { "logger: logger already exists", logger.sourceRegion };
        }
        return std::nullopt;
    }
}
