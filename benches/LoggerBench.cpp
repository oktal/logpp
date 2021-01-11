#include <benchmark/benchmark.h>

#include "logpp/logpp.h"

#include "logpp/core/LoggerFactory.h"
#include "logpp/core/Logger.h"

#include "logpp/format/PatternFormatter.h"

#include "logpp/queue/AsyncQueuePoller.h"

#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/Sink.h"

#include <random>

class NoopSink : public logpp::sink::Sink
{
public:
    void format(std::string_view, logpp::LogLevel, const logpp::EventLogBuffer&) override
    {}
};

class PatternFormatSink : public logpp::sink::Sink
{
public:
    PatternFormatSink()
        : m_formatter(std::make_shared<logpp::PatternFormatter>("%+"))
    {}

    void setPattern(std::string pattern)
    {
        m_formatter->setPattern(std::move(pattern));
    }

    void format(std::string_view name, logpp::LogLevel level, const logpp::EventLogBuffer& buffer) override
    {
        fmt::memory_buffer out;
        m_formatter->format(name, level, buffer, out);
    }

private:
    std::shared_ptr<logpp::PatternFormatter> m_formatter;
};

std::string generateRandomString(size_t size)
{
    static constexpr const char Chars[] = "abcdefghijklmnopqrstuvxyz";

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(1, std::strlen(Chars));

    std::string ret;
    ret.reserve(size);

    for (size_t i = 0; i < size; ++i)
    {
        ret.push_back(Chars[dist(gen)]);
    }

    return ret;
}

static void BM_BenchLoggerNoopSink_Empty(benchmark::State& state)
{
    auto logger = logpp::create<NoopSink>("BM_BenchLoggerNoopSink_Empty", logpp::LogLevel::Debug);

    for (auto _: state)
    {
        logger->debug("Looping");
    }
}

static void BM_BenchLoggerNoopSink_NoCopy_1(benchmark::State& state)
{
    auto logger = logpp::create<NoopSink>("BM_BenchLoggerNoopSink_NoCopy_1", logpp::LogLevel::Debug);
    uint64_t count = 0;

    for (auto _: state)
    {
        logger->debug("Looping", logpp::field("Iteration", count));
        ++count;
    }
}

static void BM_BenchLoggerNoopSink_NoCopy_FormatStr_1(benchmark::State& state)
{
    auto logger = logpp::create<NoopSink>("BM_BenchLoggerNoopSink_NoCopy_1", logpp::LogLevel::Debug);
    uint64_t count = 0;

    for (auto _: state)
    {
        logger->debug(logpp::format("Looping iteration number {}", count), logpp::field("Iteration", count));
        ++count;
    }
}

static void BM_BenchLoggerNoopSink_Copy_1(benchmark::State& state)
{
    auto logger = logpp::create<NoopSink>("BM_BenchLoggerNoopSink_Copy_1", logpp::LogLevel::Debug);
    uint64_t count = 0;

    for (auto _: state)
    {
        logger->debug(std::string_view("Looping"), logpp::field("Iteration", count));
        ++count;
    }
}

static void BM_BenchLoggerNoopSink_NoCopy_2(benchmark::State& state)
{
    auto logger = logpp::create<NoopSink>("BM_BenchLoggerNoopSink_NoCopy_2", logpp::LogLevel::Debug);
    uint64_t i = 0;

    for (auto _: state)
    {
        logger->debug("Looping",
            logpp::field("Int", i),
            logpp::field("Double", double(i))
        );

        ++i;
    }
}

static void BM_BenchLoggerNoopSink_NoCopy_3(benchmark::State& state)
{
    auto logger = logpp::create<NoopSink>("BM_BenchLoggerNoopSink_NoCopy_3", logpp::LogLevel::Debug);
    uint64_t i = 0;

    std::string name = "BM_BenchLoggerNoopSink3";

    for (auto _: state)
    {
        logger->debug("Looping",
            logpp::field("Int", i),
            logpp::field("Double", double(i)),
            logpp::field("BenchmarkName", name)
        );

        ++i;
    }
}

static void BM_BenchLoggerNoopSink_NoCopy_LargeLogBuffer(benchmark::State& state)
{
    auto logger = logpp::create<NoopSink>("BM_BenchLoggerNoopSink_NoCopy_LargeLogBuffer", logpp::LogLevel::Debug);

    auto str1 = generateRandomString(10);
    auto str2 = generateRandomString(20);
    auto str3 = generateRandomString(40);
    auto str4 = generateRandomString(512);

    for (auto _: state)
    {
        logger->debug("This is a long log message from benchmark",
            logpp::field("Str1", str1),
            logpp::field("Str2", str2),
            logpp::field("Str3", str3),
            logpp::field("Str4", str4)
        );
    }
}

static void BM_BenchLoggerFormatSink_FormatStr_3(benchmark::State& state)
{
    auto logger = logpp::create<PatternFormatSink>("BM_BenchLoggerFormatSink_StrFormat_3", logpp::LogLevel::Debug);

    const std::string& loggerType = "async";

    for (auto _: state)
    {
        logger->debug(logpp::format("This is a log-formatted message {} {}", 0xBAD, loggerType),
            logpp::field("IntField", 0xBAD),
            logpp::field("FloatField", M_PI),
            logpp::field("StrField", loggerType)
        );
    }
}

static void BM_BenchAsyncLoggerNoopSink(benchmark::State& state)
{
    auto poller = logpp::AsyncQueuePoller::create();
    auto noopSink = std::make_shared<NoopSink>();
    auto logger = logpp::create<logpp::sink::AsyncSink>("BM_BenchAsyncLoggerNoopSink", logpp::LogLevel::Debug, poller, noopSink);

    poller->start();

    const std::string& loggerType = "async";

    for (auto _: state)
    {
        logger->debug(logpp::format("This is a log-formatted message {} {}", 0xBAD, loggerType),
            logpp::field("IntField", 0xBAD),
            logpp::field("FloatField", M_PI),
            logpp::field("StrField", loggerType)
        );
    }
}

BENCHMARK(BM_BenchLoggerNoopSink_Empty);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_1);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_FormatStr_1);
BENCHMARK(BM_BenchLoggerNoopSink_Copy_1);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_2);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_3);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_LargeLogBuffer);

BENCHMARK(BM_BenchLoggerFormatSink_FormatStr_3);

BENCHMARK(BM_BenchAsyncLoggerNoopSink);

BENCHMARK_MAIN();