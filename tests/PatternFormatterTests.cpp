#include "logpp/format/PatternFormatter.h"

#include "logpp/core/Clock.h"
#include "logpp/core/Logger.h"
#include "logpp/sinks/Sink.h"

#include "logpp/utils/date.h"

#include <gtest/gtest.h>

using namespace logpp;
using namespace date;

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

    auto ymd = jan/8/2021;
    TimePoint tp = sys_days(ymd);

    EventLogBuffer buffer;
    buffer.writeTime(tp);

    format("", LogLevel::Debug, buffer);

    ASSERT_EQ(data(), "2021-01-08");
}

TEST_F(PatternFormatterTest, should_format_time)
{
    using namespace std::chrono;
    setPattern("%H:%M:%S");

    auto time = make_time(hours{15} + minutes{20} + seconds{10});

    EventLogBuffer buffer;
    buffer.writeTime(TimePoint{seconds(time)});

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

    ASSERT_EQ(data(), fmt::format("[{}]", levelString(LogLevel::Info)));
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

TEST_F(PatternFormatterTest, should_format_source_location)
{
    setPattern("%p:%o");

    EventLogBuffer buffer;
    buffer.writeSourceLocation(SourceLocation { "my/test/directory/PatternFormatterTests.cpp", 124 });

    format("", LogLevel::Info, buffer);

    ASSERT_EQ(data(), "PatternFormatterTests.cpp:124");
}

TEST_F(PatternFormatterTest, should_format_full)
{
    using namespace std::chrono;
    setPattern("%+");

    auto ymd = jan/8/2021;
    TimePoint tp = sys_days(ymd) + hours{15} + minutes{20} + seconds{10};

    EventLogBuffer buffer;
    buffer.writeTime(tp);
    buffer.writeText(logpp::format("Test result: {} ({})", std::string("Pass"), 0));

    format("MyLogger", LogLevel::Info, buffer);

    ASSERT_EQ(
        data(),
        fmt::format("2021-01-08 15:20:10 [{}] (MyLogger) Test result: Pass (0)", levelString(LogLevel::Info))
    );
}
