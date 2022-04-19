#include <gtest/gtest.h>

#include "logpp/config/TomlConfigurator.h"
#include "logpp/sinks/ColoredConsole.h"
#include "logpp/sinks/file/FileSink.h"
#include "logpp/utils/env.h"

#include "TemporaryFile.h"

using namespace std::string_view_literals;
using namespace logpp;

namespace
{
    struct HasLevel
    {
        explicit HasLevel(LogLevel level)
            : level(level)
        { }

        LogLevel level;

        void operator()(const std::shared_ptr<Logger>& logger)
        {
            ASSERT_EQ(logger->level(), level)
                << "expected " << levelString(level) << "but got " << levelString(logger->level());
        }
    };

    template <typename Sink>
    struct HasSink
    {
        static_assert(sink::concepts::IsSink<Sink>, "Sink type does not satisfy Sink concept");

        void operator()(const std::shared_ptr<Logger>& logger)
        {
            auto targetSink = std::dynamic_pointer_cast<Sink>(logger->sink());
            ASSERT_TRUE(targetSink) << "expected sink of type " << Sink::Name;
        }
    };

    template <typename... Checks>
    void checkLogger(LoggerRegistry& registry, std::string_view name, Checks&&... checks)
    {
        auto logger = registry.get(name);
        EXPECT_TRUE(logger);
        ASSERT_EQ(logger->name(), name);

        (std::invoke(std::forward<Checks>(checks), logger), ...);
    }
}

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

    checkLogger(registry, "My.Namespace.Class", HasLevel(LogLevel::Info));
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

    checkLogger(registry, "My.Namespace.Class", HasLevel(LogLevel::Debug));
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

TEST(TomlConfigurator, should_register_default_logger)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [loggers]
        [loggers.default]
           name = "default"
           sinks = [ "console" ]
           level = "info"
           default = true

        [loggers.Tests]
           name = "TomlConfiguratorTests"
           sinks = [ "console" ]
           level = "debug"
    )TOML"sv;

    LoggerRegistry registry;
    auto err = TomlConfigurator::configure(Config, registry);
    ASSERT_FALSE(err) << *err;

    checkLogger(registry, "UnregisteredLogger", HasLevel(LogLevel::Info));
    checkLogger(registry, "TomlConfiguratorTests", HasLevel(LogLevel::Debug));
}

TEST(TomlConfigurator, should_configure_hierarichal_loggers_and_use_sink_level_parent_properties_if_empty)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [sinks.file]
           type = "File"
           options = { file = "test.log" }

        [loggers]
        [loggers.default]
           name = "default"
           sinks = [ "console" ]
           level = "info"
           default = true

        [loggers.Namespace]
           name = "My.Namespace"
           sinks = [ "file" ]

        [loggers.Class]
           name = "My.Namespace.Class"
           level = "debug"

        [loggers.Other]
           name = "My.Other"
           level = "debug"
    )TOML"sv;

    LoggerRegistry registry;
    auto err = TomlConfigurator::configure(Config, registry);
    ASSERT_FALSE(err) << *err;

    checkLogger(registry, "My.Namespace", HasLevel(LogLevel::Info), HasSink<sink::FileSink>());
    checkLogger(registry, "My.Namespace.Class", HasLevel(LogLevel::Debug), HasSink<sink::FileSink>());
    checkLogger(registry, "My.Other", HasLevel(LogLevel::Debug), HasSink<sink::ColoredOutputConsole>());
}

TEST(TomlConfigurator, should_error_when_configuring_hierarchical_loggers_with_missing_parents)
{
    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [sinks.file]
           type = "File"
           options = { file = "test.log" }

        [loggers]
        [loggers.Namespace]
           name = "My.Namespace"
           sinks = [ "file" ]
           level = "info"

        [loggers.Class]
           name = "My.Namespace.Class"
           level = "debug"

        [loggers.Other]
           name = "My.Other"
           level = "debug"
    )TOML"sv;

    LoggerRegistry registry;
    auto err = TomlConfigurator::configure(Config, registry);
    ASSERT_TRUE(err);
}

TEST(TomlConfigurator, should_expand_environment_variable)
{
    env_utils::setenv("LOGPP_SINK_TYPE", "console", 1);
    env_utils::setenv("LOGPP_ROOT_LEVEL", "info", 1);

    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [sinks.file]
           type = "File"
           options = { file = "test.log" }

        [loggers]
        [loggers.Namespace]
           name = "My.Namespace"
           sinks = [ "${LOGPP_SINK_TYPE}" ]
           level = "${LOGPP_ROOT_LEVEL}"
           default = true

        [loggers.Class]
           name = "My.Namespace.Class"
           level = "debug"
           sinks = [ "file" ]

        [loggers.Other]
           name = "My.Other"
           level = "debug"
    )TOML"sv;

    LoggerRegistry registry;
    TomlConfigurator::Options options;
    options.expandEnvironmentVariables = true;
    auto err = TomlConfigurator::configure(Config, registry, options);
    ASSERT_FALSE(err) << *err;

    checkLogger(registry, "My.Namespace", HasLevel(LogLevel::Info), HasSink<sink::ColoredOutputConsole>());
    checkLogger(registry, "My.Namespace.Class", HasLevel(LogLevel::Debug), HasSink<sink::FileSink>());
    checkLogger(registry, "My.Other", HasLevel(LogLevel::Debug), HasSink<sink::ColoredOutputConsole>());
}

TEST(TomlConfigurator, should_expand_environment_variable_from_file)
{
    env_utils::setenv("LOGPP_SINK_TYPE", "console", 1);
    env_utils::setenv("LOGPP_ROOT_LEVEL", "info", 1);

    static constexpr auto Config = R"TOML(
        [sinks]
        [sinks.console]
           type = "ColoredOutputConsole" 
           options = { pattern = "%+" }

        [sinks.file]
           type = "File"
           options = { file = "test.log" }

        [loggers]
        [loggers.Namespace]
           name = "My.Namespace"
           sinks = [ "${LOGPP_SINK_TYPE}" ]
           level = "${LOGPP_ROOT_LEVEL}"
           default = true

        [loggers.Class]
           name = "My.Namespace.Class"
           level = "debug"
           sinks = [ "file" ]

        [loggers.Other]
           name = "My.Other"
           level = "debug"
    )TOML"sv;

    temporary_ofstream ofs(std::ios_base::out, "logpptestconfig", ".toml");
    ofs.write(Config.data(), Config.size());
    ofs.flush();

    RemoveDirectoryOnExit rmDir(ofs.directory());

    LoggerRegistry registry;
    TomlConfigurator::Options options;
    options.expandEnvironmentVariables = true;
    auto err = TomlConfigurator::configureFile(ofs.path(), registry, options);
    ASSERT_FALSE(err) << *err;

    checkLogger(registry, "My.Namespace", HasLevel(LogLevel::Info), HasSink<sink::ColoredOutputConsole>());
    checkLogger(registry, "My.Namespace.Class", HasLevel(LogLevel::Debug), HasSink<sink::FileSink>());
    checkLogger(registry, "My.Other", HasLevel(LogLevel::Debug), HasSink<sink::ColoredOutputConsole>());
}
