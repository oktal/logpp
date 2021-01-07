#include "gtest/gtest.h"

#include "logpp/core/LoggerRegistry.h"
#include "logpp/sinks/Sink.h"

using namespace logpp;

class NoopSink : public sink::Sink
{
public:
    void format(std::string_view, LogLevel, EventLogBuffer, StringOffset)
    { }
};

std::shared_ptr<Logger> createLogger(std::string name)
{
    return std::make_shared<Logger>(std::move(name), LogLevel::Debug, std::make_shared<NoopSink>());
}

TEST(LoggerKey, should_iterate_over_key)
{
    using namespace std::string_view_literals;
    LoggerKey key("My.Namespace.Class"sv);

    auto it = key.begin();
    for (auto fragment: { "My"sv, "My.Namespace"sv, "My.Namespace.Class"sv })
    {
        ASSERT_EQ(*it, fragment);
        ++it;
    }

    ASSERT_EQ(it, key.end());
}

TEST(LoggerKey, should_iterate_over_key_in_reverse_order)
{
    using namespace std::string_view_literals;
    LoggerKey key("My.Namespace.Class"sv);

    auto it = key.rbegin();
    for (auto fragment: { "My.Namespace.Class"sv, "My.Namespace"sv, "My"sv })
    {
        ASSERT_EQ(*it, fragment);
        ++it;
    }

    ASSERT_EQ(it, key.rend());
}

TEST(LoggerRegistry, should_register_logger)
{
    LoggerRegistry registry;
    ASSERT_TRUE(registry.registerLogger(createLogger("TestLogger")));
}

TEST(LoggerRegistry, should_not_register_if_already_exists)
{
    LoggerRegistry registry;
    ASSERT_TRUE(registry.registerLogger(createLogger("TestLogger")));
    ASSERT_FALSE(registry.registerLogger(createLogger("TestLogger")));
}

TEST(LoggerRegistry, should_retrieve_logger)
{
    LoggerRegistry registry;
    ASSERT_TRUE(registry.registerLogger(createLogger("TestLogger")));

    auto logger = registry.get("TestLogger");
    ASSERT_NE(logger, nullptr);
    ASSERT_EQ(logger->name(), "TestLogger");
    ASSERT_EQ(logger->level(), LogLevel::Debug);
}

TEST(LoggerRegistry, should_retrieve_parent_logger_from_children)
{
    LoggerRegistry registry;
    auto parentLogger = createLogger("My.Namespace");
    ASSERT_TRUE(registry.registerLogger(parentLogger));

    auto logger = registry.get("My.Namespace.Class");
    ASSERT_EQ(logger, parentLogger);
}

TEST(LoggerRegistry, should_retrieve_child_logger)
{
    LoggerRegistry registry;
    auto parentLogger = createLogger("My.Namespace");
    ASSERT_TRUE(registry.registerLogger(parentLogger));

    auto childLogger = createLogger("My.Namespace.Class");
    ASSERT_TRUE(registry.registerLogger(childLogger));

    auto logger = registry.get("My.Namespace.Class");
    ASSERT_EQ(logger, childLogger);
}