#pragma once

#include "logpp/core/LoggerRegistry.h"

#include <toml.hpp>

namespace logpp
{

class TomlConfigurator
{
public:
    struct Error
    {
        std::string_view description;
        toml::source_region region;

        static Error from(const toml::parse_error& e)
        {
            return { e.description(), e.source() };
        }

        friend std::ostream& operator<<(std::ostream& os, const Error& err)
        {
            os << err.description << ' ' << err.region;
            return os;
        }
    };

    static std::optional<Error> configure(std::string_view config);
    static std::optional<Error> configure(std::string_view config, LoggerRegistry& registry);

private:
    struct Sink
    {
        std::string name;
        std::string type;
        std::unordered_map<std::string, std::string> options;

        friend std::ostream& operator<<(std::ostream& os, const Sink& sink)
        {
            os << "Sink(name=" << sink.name << ", type=" << sink.type << ", options={";
            size_t i = 0;
            for (const auto& option: sink.options)
            {
                if (i > 0)
                    os << ", ";
                os << option.first << "=" << option.second;
                ++i;
            }
            os << "})";
            return os;
        }
    };

    static std::pair<std::vector<Sink>, std::optional<Error>> parseSinks(const toml::table& table);
    static std::pair<std::optional<Sink>, std::optional<Error>> parseSink(std::string name, const toml::table& table);

    static std::optional<Error> parseLoggers(const toml::table& table, const std::vector<Sink>& sinks, LoggerRegistry& registry);
    static std::optional<Error> parseLogger(std::string tableName, const toml::table& table, const std::vector<Sink>& sinks, LoggerRegistry& registry);

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