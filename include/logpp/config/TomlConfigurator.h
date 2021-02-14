#pragma once

#include "logpp/core/LoggerRegistry.h"
#include "logpp/config/FileWatcher.h"

#include <toml.hpp>
#include <istream>

namespace logpp
{

class TomlConfigurator
{
public:
    struct Error
    {
        std::string_view description;
        std::optional<toml::source_region> region;

        static Error from(const toml::parse_error& e)
        {
            return { e.description(), e.source() };
        }

        friend std::ostream& operator<<(std::ostream& os, const Error& err)
        {
            os << err.description;
            if (err.region)
                os << ' ' << err.region.value();
            return os;
        }
    };

    static std::optional<Error> configure(std::string_view config);
    static std::optional<Error> configure(std::string_view config, LoggerRegistry& registry);

    static std::optional<Error> configureFile(std::string_view path);
    static std::optional<Error> configureFile(std::string_view path, LoggerRegistry& registry);

    static std::pair<std::optional<FileWatcher::WatchId>, std::optional<Error>>
    configureFileAndWatch(std::string_view path, std::shared_ptr<FileWatcher> watcher);

    static std::pair<std::optional<FileWatcher::WatchId>, std::optional<Error>>
    configureFileAndWatch(std::string_view path, std::shared_ptr<FileWatcher> watcher, LoggerRegistry& registry);

private:
    template<typename ParseFunc>
    static std::optional<Error> configure(ParseFunc parseFunc, LoggerRegistry& registry)
    {
        try
        {
            auto table = std::invoke(parseFunc);
            auto [loggers, err] = parseConfiguration(table);
            if (err)
                return err;
            return configureRegistry(registry, loggers);
        }
        catch (const toml::parse_error& e)
        {
            return Error::from(e);
        }
        
        return std::nullopt;
    }

    struct Sink
    {
        std::string name;
        std::string type;
        sink::Options options;

        toml::source_region sourceRegion;
    };

    struct Logger
    {
        std::string name;
        LogLevel level;
        std::vector<Sink> sinks;

        toml::source_region sourceRegion;
    };

    static std::pair<std::vector<Logger>, std::optional<Error>> parseConfiguration(const toml::table& table);

    static std::pair<std::vector<Sink>, std::optional<Error>> parseSinks(const toml::table& table);
    static std::pair<std::optional<Sink>, std::optional<Error>> parseSink(std::string name, const toml::table& table);

    static std::pair<std::vector<Logger>, std::optional<Error>> parseLoggers(const toml::table& table, const std::vector<Sink>& sinks);
    static std::pair<std::optional<Logger>, std::optional<Error>> parseLogger(std::string tableName, const toml::table& table, const std::vector<Sink>& sinks);

    static std::optional<Error> configureRegistry(LoggerRegistry& registry, const std::vector<Logger>& loggers);

    template<typename T>
    static std::pair<std::optional<T>, std::optional<Error>>
    tryRead(const toml::table& table, std::string_view key, std::string_view error)
    {
        auto val = table[key].value<T>();

        if (!val)
            return std::make_pair(std::nullopt, Error{error, table.source()});

        return std::make_pair(std::move(val), std::nullopt);
    }
};

}