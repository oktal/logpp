#include "logpp/format/LogFmtFormatter.h"

#include "logpp/core/Clock.h"
#include "logpp/core/Logger.h"

#include "logpp/utils/date.h"

#include <gtest/gtest.h>

using namespace logpp;
using namespace date;

struct LogFmtFormatterTest : public ::testing::Test
{
    std::unique_ptr<LogFmtFormatter> formatter;

    void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        m_out.clear();
        formatter->format(name, level, buffer, m_out);
    }

    void setPattern(std::string pattern)
    {
        formatter = std::make_unique<LogFmtFormatter>(std::move(pattern));
    }

    std::string data() const
    {
        return std::string(m_out.data(), m_out.size());
    }

private:
    fmt::memory_buffer m_out;
};

TEST_F(LogFmtFormatterTest, should_error_on_invalid_pattern)
{
    EXPECT_THROW(setPattern("ts="), LogFmtPatternError);
    EXPECT_THROW(setPattern("ts=%K"), LogFmtPatternError);
}

TEST_F(LogFmtFormatterTest, should_format_date)
{
    setPattern("ts=%Y-%m-%d");

    auto ymd     = jan / 8 / 2021;
    TimePoint tp = sys_days(ymd);

    EventLogBuffer buffer;
    buffer.writeTime(tp);

    format("", LogLevel::Debug, buffer);

    ASSERT_EQ(data(), "ts=2021-01-08");
}

TEST_F(LogFmtFormatterTest, should_format_time)
{
    using namespace std::chrono;
    setPattern("ts=%H:%M:%S");

    auto time = make_time(hours { 15 } + minutes { 20 } + seconds { 10 });

    EventLogBuffer buffer;
    buffer.writeTime(TimePoint { seconds(time) });

    format("", LogLevel::Debug, buffer);

    ASSERT_EQ(data(), "ts=15:20:10");
}

TEST_F(LogFmtFormatterTest, should_format_text)
{
    setPattern("msg=%v");

    {
        EventLogBuffer buffer;
        buffer.writeText("Message with spaces");

        format("", LogLevel::Debug, buffer);

        ASSERT_EQ(data(), "msg=\"Message with spaces\"");
    }

    {
        EventLogBuffer buffer;
        buffer.writeText("Message");

        format("", LogLevel::Debug, buffer);

        ASSERT_EQ(data(), "msg=Message");
    }
}

TEST_F(LogFmtFormatterTest, should_format_level)
{
    setPattern("lvl=%l");

    EventLogBuffer buffer;
    format("", LogLevel::Info, buffer);

    ASSERT_EQ(data(), fmt::format("lvl={}", levelString(LogLevel::Info)));
}

TEST_F(LogFmtFormatterTest, should_format_name)
{
    setPattern("logger=%n");

    EventLogBuffer buffer;

    format("MyLogger", LogLevel::Info, buffer);
    ASSERT_EQ(data(), "logger=MyLogger");

    format("MyLogger with spaces", LogLevel::Info, buffer);
    ASSERT_EQ(data(), "logger=\"MyLogger with spaces\"");
}

TEST_F(LogFmtFormatterTest, should_format_source_location)
{
    setPattern("src=%p:%o");

    EventLogBuffer buffer;
    buffer.writeSourceLocation(SourceLocation { "my/test/directory/LogFmtFormatterTests.cpp", 124 });

    format("", LogLevel::Info, buffer);
    ASSERT_EQ(data(), "src=LogFmtFormatterTests.cpp:124");
}

TEST_F(LogFmtFormatterTest, should_format_full)
{
    using namespace std::chrono;
    setPattern("%+");

    auto ymd     = jan / 8 / 2021;
    TimePoint tp = sys_days(ymd) + hours { 15 } + minutes { 20 } + seconds { 10 };

    EventLogBuffer buffer;
    buffer.writeTime(tp);
    buffer.writeText(logpp::format("Test result: {} ({})", std::string("Pass"), 0));

    format("MyLogger", LogLevel::Info, buffer);

    ASSERT_EQ(
        data(),
        fmt::format("ts=2021-01-08T15:20:10 lvl={} logger=MyLogger msg=\"Test result: Pass (0)\"", levelString(LogLevel::Info)));
}

TEST_F(LogFmtFormatterTest, should_format_fields)
{
    setPattern("msg=%v%f");

    EventLogBuffer buffer;
    buffer.writeText("Test message");
    buffer.writeFields(
        logpp::field("test_name", "should_format_fields"),
        logpp::field("test_success", true));

    format("", LogLevel::Info, buffer);

    ASSERT_EQ(data(), "msg=\"Test message\" test_name=should_format_fields test_success=true");
}

TEST_F(LogFmtFormatterTest, should_format_constant_fields)
{
    setPattern("lvl=%l region=eu-west-3");

    EventLogBuffer buffer;
    format("MyLogger", LogLevel::Info, buffer);

    ASSERT_EQ(data(), fmt::format("lvl={} region=eu-west-3", levelString(LogLevel::Info)));
}
