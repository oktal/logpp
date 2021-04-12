#include <benchmark/benchmark.h>

#include "logpp/core/Logger.h"

#include "logpp/format/PatternFormatter.h"

#include "logpp/queue/AsyncQueuePoller.h"

#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/FormatSink.h"
#include "logpp/sinks/Sink.h"

#include <random>

class NoopSink : public logpp::sink::Sink
{
public:
    void activateOptions(const logpp::sink::Options&) override
    {
    }

    void sink(std::string_view, logpp::LogLevel, const logpp::EventLogBuffer&) override
    { }
};

class PatternFormatSink : public logpp::sink::FormatSink
{
public:
    PatternFormatSink()
        : FormatSink(std::make_shared<logpp::PatternFormatter>("%+"))
    { }

    void sink(std::string_view name, logpp::LogLevel level, const logpp::EventLogBuffer& buffer) override
    {
        fmt::memory_buffer out;
        format(name, level, buffer, out);
    }
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

template<typename Sink, typename... Args>
std::shared_ptr<logpp::Logger> create(std::string name, logpp::LogLevel level, Args&& ...args)
{
    auto sink = std::make_shared<Sink>(std::forward<Args>(args)...);
    return std::make_shared<logpp::Logger>(std::move(name), level, std::move(sink));
}

static void LoggerBench_NoopSink_Empty(benchmark::State& state)
{
    auto logger = create<NoopSink>("LoggerBench_NoopSink_Empty", logpp::LogLevel::Debug);

    for (auto _ : state)
    {
        logger->debug("Looping");
    }
}

static void LoggerBench_NoopSink_StringLiteral_1(benchmark::State& state)
{
    auto logger    = create<NoopSink>("LoggerBench_NoopSink_StringLiteral_1", logpp::LogLevel::Debug);
    uint64_t count = 0;

    for (auto _ : state)
    {
        logger->debug("Looping", logpp::field("Iteration", count));
        ++count;
    }
}

static void LoggerBench_NoopSink_FormatStr_1(benchmark::State& state)
{
    auto logger    = create<NoopSink>("LoggerBench_NoopSink_FormatStr_1", logpp::LogLevel::Debug);
    uint64_t count = 0;

    for (auto _ : state)
    {
        logger->debug(logpp::format("Looping iteration number {}", count), logpp::field("Iteration", count));
        ++count;
    }
}

static void LoggerBench_NoopSink_1(benchmark::State& state)
{
    auto logger    = create<NoopSink>("LoggerBench_NoopSink_1", logpp::LogLevel::Debug);
    uint64_t count = 0;

    for (auto _ : state)
    {
        logger->debug(std::string_view("Looping"), logpp::field("Iteration", count));
        ++count;
    }
}

static void LoggerBench_NoopSink_StringLiteral_2(benchmark::State& state)
{
    auto logger = create<NoopSink>("LoggerBench_NoopSink_StringLiteral_2", logpp::LogLevel::Debug);
    uint64_t i  = 0;

    for (auto _ : state)
    {
        logger->debug("Looping",
                      logpp::field("Int", i),
                      logpp::field("Double", double(i)));

        ++i;
    }
}

static void LoggerBench_NoopSink_StringLiteral_3(benchmark::State& state)
{
    auto logger = create<NoopSink>("LoggerBench_NoopSink_StringLiteral_3", logpp::LogLevel::Debug);
    uint64_t i  = 0;

    std::string name = "LoggerBench_NoopSink3";

    for (auto _ : state)
    {
        logger->debug("Looping",
                      logpp::field("Int", i),
                      logpp::field("Double", double(i)),
                      logpp::field("BenchmarkName", name));

        ++i;
    }
}

static void LoggerBench_NoopSink_StringLiteral_LargeLogBuffer(benchmark::State& state)
{
    auto logger = create<NoopSink>("LoggerBench_NoopSink_StringLiteral_LargeLogBuffer", logpp::LogLevel::Debug);

    auto str1 = generateRandomString(10);
    auto str2 = generateRandomString(20);
    auto str3 = generateRandomString(40);
    auto str4 = generateRandomString(512);

    for (auto _ : state)
    {
        logger->debug("This is a long log message from benchmark",
                      logpp::field("Str1", str1),
                      logpp::field("Str2", str2),
                      logpp::field("Str3", str3),
                      logpp::field("Str4", str4));
    }
}

static void LoggerBench_FormatSink_FormatStr_3(benchmark::State& state)
{
    auto logger = create<PatternFormatSink>("LoggerBench_FormatSink_StrFormat_3", logpp::LogLevel::Debug);

    const std::string& loggerType = "async";

    for (auto _ : state)
    {
        logger->debug(logpp::format("This is a log-formatted message {} {}", 0xBAD, loggerType),
                      logpp::field("IntField", 0xBAD),
                      logpp::field("FloatField", M_PI),
                      logpp::field("StrField", loggerType));
    }
}

static void LoggerBench_AsyncNoopSink_FormatStr_3(benchmark::State& state)
{
    auto poller   = logpp::AsyncQueuePoller::create();
    auto noopSink = std::make_shared<NoopSink>();
    auto logger   = create<logpp::sink::AsyncSink>("BM_BenchAsyncLoggerNoopSink", logpp::LogLevel::Debug, poller, noopSink);

    poller->start();

    const std::string& loggerType = "async";

    for (auto _ : state)
    {
        logger->debug(logpp::format("This is a log-formatted message {} {}", 0xBAD, loggerType),
                      logpp::field("IntField", 0xBAD),
                      logpp::field("FloatField", M_PI),
                      logpp::field("StrField", loggerType));
    }
}

BENCHMARK(LoggerBench_NoopSink_Empty);
BENCHMARK(LoggerBench_NoopSink_StringLiteral_1);
BENCHMARK(LoggerBench_NoopSink_FormatStr_1);
BENCHMARK(LoggerBench_NoopSink_1);
BENCHMARK(LoggerBench_NoopSink_StringLiteral_2);
BENCHMARK(LoggerBench_NoopSink_StringLiteral_3);
BENCHMARK(LoggerBench_NoopSink_StringLiteral_LargeLogBuffer);

BENCHMARK(LoggerBench_FormatSink_FormatStr_3);

BENCHMARK(LoggerBench_AsyncNoopSink_FormatStr_3);

BENCHMARK_MAIN();
