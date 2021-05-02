#pragma once

#include "logpp/config/FileWatcher.h"
#include "logpp/core/LoggerRegistry.h"

#include <istream>
#include <toml.hpp>

namespace logpp
{

    class TomlConfigurator
    {
    public:
        struct Error
        {
            std::string description;
            std::optional<toml::source_region> region;

            static Error from(const toml::parse_error& e)
            {
                return { std::string(e.description()), e.source() };
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
        template <typename ParseFunc>
        static std::optional<Error> configure(ParseFunc parseFunc, LoggerRegistry& registry)
        {
            try
            {
                auto table = std::invoke(parseFunc);

                auto result = parseConfiguration(table);
                auto err    = result.error;

                if (err)
                    return err;

#define TRY(...)                 \
    do                           \
    {                            \
        auto _err = __VA_ARGS__; \
        if (_err)                \
            return _err;         \
    } while (0)

                TRY(prepareLoggers(result.loggers));
                TRY(configureSinks(registry, result.sinks));
                TRY(configureLoggers(registry, result.loggers));

#undef TRY
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
            std::optional<LogLevel> level;
            sink::Options options;

            toml::source_region sourceRegion;
        };

        struct Logger
        {
            std::string name;

            std::optional<LogLevel> level { std::nullopt };
            std::vector<Sink> sinks;

            bool isDefault { false };

            toml::source_region sourceRegion;

            bool hasMissing() const
            {
                return !level.has_value() || sinks.empty();
            }
        };

        struct ParseResult
        {
            std::vector<Sink> sinks;
            std::vector<Logger> loggers;

            std::optional<Error> error;

            static ParseResult success(std::vector<Sink> sinks, std::vector<Logger> loggers)
            {
                return ParseResult {
                    std::move(sinks),
                    std::move(loggers),
                    std::nullopt
                };
            }

            static ParseResult failure(Error error)
            {
                return ParseResult {
                    std::vector<Sink> {},
                    std::vector<Logger> {},
                    error
                };
            }
        };

        static ParseResult parseConfiguration(const toml::table& table);

        static std::pair<std::vector<Sink>, std::optional<Error>> parseSinks(const toml::table& table);
        static std::pair<std::optional<Sink>, std::optional<Error>> parseSink(std::string name, const toml::table& table);

        static std::pair<std::vector<Logger>, std::optional<Error>> parseLoggers(const toml::table& table, const std::vector<Sink>& sinks);
        static std::pair<std::optional<Logger>, std::optional<Error>> parseLogger(std::string tableName, const toml::table& table, const std::vector<Sink>& sinks);

        static std::optional<Logger> findParent(const Logger& logger, const std::vector<Logger>& loggers);
        static std::optional<Error> prepareLoggers(std::vector<Logger>& loggers);

        static std::optional<TomlConfigurator::Error> configureSinks(LoggerRegistry& registry, std::vector<Sink> sinks);
        static std::optional<Error> configureLoggers(LoggerRegistry& registry, const std::vector<Logger>& loggers);

        template <typename T>
        static std::pair<std::optional<T>, std::optional<Error>>
        tryRead(const toml::table& table, std::string_view key, std::string_view error)
        {
            auto val = table[key].value<T>();

            if (!val)
                return std::make_pair(std::nullopt, Error { std::string(error), table.source() });

            return std::make_pair(std::move(val), std::nullopt);
        }

        template <typename T>
        static std::pair<std::optional<T>, std::optional<Error>>
        tryReadOr(const toml::table& table, std::string_view key, std::string_view error, T defaultValue)
        {
            if (!table.contains(key))
                return std::make_pair(defaultValue, std::nullopt);

            auto val = table[key].value<T>();

            if (!val)
                return std::make_pair(std::nullopt, Error { std::string(error), table.source() });

            return std::make_pair(std::move(val), std::nullopt);
        }
    };
}
