#include <gtest/gtest.h>

#include "logpp/config/TomlConfigurator.h"

using namespace std::string_view_literals;
using namespace logpp;

TEST(TomlConfigurator, should_error_on_empty_logger_name)
{
    static constexpr auto Config = R"TOML(
        [loggers]
        [loggers.MyLogger]
           sinks = [ "console" ]
           level = "info"
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_error_on_invalid_logger_name)
{
    static constexpr auto Config = R"TOML(
        [loggers]
        [loggers.MyLogger]
           name = 0
           sinks = [ "console" ]
           level = "info"
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_error_on_empty_logger_level)
{
    static constexpr auto Config = R"TOML(
        [loggers]
        [loggers.MyLogger]
           name = "My.Namespace"
           sinks = [ "console" ]
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_error_on_invalid_logger_level)
{
    static constexpr auto Config = R"TOML(
        [loggers]
        [loggers.MyLogger]
           name = "My.Namespace"
           sinks = [ "console" ]
           level = 2
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_error_on_unknown_logger_level)
{
    static constexpr auto Config = R"TOML(
        [loggers]
        [loggers.MyLogger]
           name = "My.Namespace"
           sinks = [ "console" ]
           level = "notalevel"
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_error_on_empty_sink_type)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           options = { pattern = "%+" }
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_error_on_unknown_sink_type)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "notatype"
           options = { pattern = "%+" }
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_error_on_invalid_logger_sink_ref)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [loggers]
        [loggers.MyLogger]
           name = "My.Namespace"
           sinks = [ "notasink" ]
           level = "info"
    )TOML"sv;

    LoggerRegistry registry;
    ASSERT_TRUE(TomlConfigurator::configure(Config, registry));
}

TEST(TomlConfigurator, should_configure_and_register_logger)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [loggers]
        [loggers.Namespace]
           name = "My.Namespace"
           sinks = [ "console" ]
           level = "info"
    )TOML"sv;

    LoggerRegistry registry;
    auto err = TomlConfigurator::configure(Config, registry);
    ASSERT_FALSE(err) << *err;

    auto logger = registry.get("My.Namespace.Class");
    ASSERT_EQ(std::string(logger->name()), "My.Namespace.Class");
    ASSERT_EQ(logger->level(), LogLevel::Info);
}

TEST(TomlConfigurator, should_configure_and_register_hierarchical_logger)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [loggers]
        [loggers.Namespace]
           name = "My.Namespace"
           sinks = [ "console" ]
           level = "info"

        [loggers.Class]
           name = "My.Namespace.Class"
           sinks = [ "console" ]
           level = "debug"
    )TOML"sv;

    LoggerRegistry registry;
    auto err = TomlConfigurator::configure(Config, registry);
    ASSERT_FALSE(err) << *err;

    auto logger = registry.get("My.Namespace.Class");
    ASSERT_EQ(std::string(logger->name()), "My.Namespace.Class");
    ASSERT_EQ(logger->level(), LogLevel::Debug);
}

TEST(TomlConfigurator, should_error_on_loggers_with_same_name)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [loggers]
        [loggers.Namespace]
           name = "My.Namespace"
           sinks = [ "console" ]
           level = "info"

        [loggers.Class]
           name = "My.Namespace"
           sinks = [ "console" ]
           level = "debug"
    )TOML"sv;

    LoggerRegistry registry;
    auto err = TomlConfigurator::configure(Config, registry);
    ASSERT_TRUE(err);
}