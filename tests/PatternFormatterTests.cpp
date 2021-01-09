#include "logpp/format/PatternFormatter.h"

#include "logpp/core/Clock.h"
#include "logpp/core/Logger.h"
#include "logpp/sinks/Sink.h"

#include <gtest/gtest.h>

using namespace logpp;

struct PatternFormatterTest : public ::testing::Test
{
    std::unique_ptr<PatternFormatter> formatter;

    void format(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        formatter->format(name, level, buffer, m_out);
    }

    void setPattern(std::string pattern)
    {
        formatter = std::make_unique<PatternFormatter>(std::move(pattern));
    }

    std::string_view data() const
    {
        return std::string_view(m_out.data(), m_out.size());
    }

private:
    fmt::memory_buffer m_out;
};

TEST_F(PatternFormatterTest, should_format_date)
{
    setPattern("%Y-%m-%d");

    std::tm date;
    std::memset(&date, 0, sizeof(date));

    date.tm_year = 2021 - 1900;
    date.tm_mon = 0;
    date.tm_mday = 8;
    date.tm_hour = 15;
    date.tm_min = 20;
    date.tm_sec = 10;

    auto time = std::mktime(&date);
    auto ts = Clock::from_time_t(time);

    EventLogBuffer buffer;
    buffer.writeTime(ts);

    format("", LogLevel::Debug, buffer);

    ASSERT_EQ(data(), "2021-01-08");
}

TEST_F(PatternFormatterTest, should_format_time)
{
    setPattern("%H:%M:%S");

    std::tm date;
    std::memset(&date, 0, sizeof(date));

    date.tm_year = 2021 - 1900;
    date.tm_mon = 0;
    date.tm_mday = 8;
    date.tm_hour = 15;
    date.tm_min = 20;
    date.tm_sec = 10;

    auto time = std::mktime(&date);
    auto ts = Clock::from_time_t(time);

    EventLogBuffer buffer;
    buffer.writeTime(ts);

    format("", LogLevel::Debug, buffer);

    ASSERT_EQ(data(), "15:20:10");
}

TEST_F(PatternFormatterTest, should_format_text)
{
    setPattern("%v");

    EventLogBuffer buffer;
    buffer.writeTime(Clock::now());
    buffer.writeText("Hello message");

    format("", LogLevel::Debug, buffer);

    ASSERT_EQ(data(), "Hello message");
}

TEST_F(PatternFormatterTest, should_format_text_formatted)
{
    setPattern("%v");

    EventLogBuffer buffer;
    buffer.writeTime(Clock::now());
    buffer.writeText(logpp::format("Test result: {} ({})", std::string("Pass"), 0));

    format("", LogLevel::Debug, buffer);

    ASSERT_EQ(data(), "Test result: Pass (0)");
}

TEST_F(PatternFormatterTest, should_format_level)
{
    setPattern("[%l]");

    EventLogBuffer buffer;
    buffer.writeTime(Clock::now());
    buffer.writeText(logpp::format("Test result: {} ({})", std::string("Pass"), 0));

    format("", LogLevel::Info, buffer);

    ASSERT_EQ(data(), "[Info]");
}

TEST_F(PatternFormatterTest, should_format_name)
{
    setPattern("[%n]");

    EventLogBuffer buffer;
    buffer.writeTime(Clock::now());
    buffer.writeText(logpp::format("Test result: {} ({})", std::string("Pass"), 0));

    format("MyLogger", LogLevel::Info, buffer);

    ASSERT_EQ(data(), "[MyLogger]");
}

TEST_F(PatternFormatterTest, should_format_full)
{
    setPattern("%+");

    std::tm date;
    std::memset(&date, 0, sizeof(date));

    date.tm_year = 2021 - 1900;
    date.tm_mon = 0;
    date.tm_mday = 8;
    date.tm_hour = 15;
    date.tm_min = 20;
    date.tm_sec = 10;

    auto time = std::mktime(&date);
    auto ts = Clock::from_time_t(time);

    EventLogBuffer buffer;
    buffer.writeTime(ts);
    buffer.writeText(logpp::format("Test result: {} ({})", std::string("Pass"), 0));

    format("MyLogger", LogLevel::Info, buffer);

    ASSERT_EQ(data(), "2021-01-08 15:20:10 [Info] (MyLogger) Test result: Pass (0)");
}