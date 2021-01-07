#include <benchmark/benchmark.h>

#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/Sink.h"

#include "logpp/core/LoggerFactory.h"
#include "logpp/core/Logger.h"

#include "logpp/queue/AsyncQueuePoller.h"

#include <random>

class NoopSink : public logpp::sink::Sink
{
public:
    void format(std::string_view name, logpp::LogLevel level, logpp::EventLogBuffer buffer, logpp::StringOffset text) override
    {}
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

static void BM_BenchLoggerNoopSink_NoCopy_1(benchmark::State& state)
{
    auto logger = logpp::LoggerFactory::getLogger("BM_BenchLoggerNoopSink_NoCopy_1", std::make_shared<NoopSink>());
    uint64_t count = 0;

    for (auto _: state)
    {
        logger->debug("Looping", logpp::data("Iteration", count));
        ++count;
    }
}

static void BM_BenchLoggerNoopSink_Copy_1(benchmark::State& state)
{
    auto logger = logpp::LoggerFactory::getLogger("BM_BenchLoggerNoopSink_Copy_1", std::make_shared<NoopSink>());
    uint64_t count = 0;

    for (auto _: state)
    {
        logger->debug(std::string_view("Looping"), logpp::data("Iteration", count));
        ++count;
    }
}

static void BM_BenchLoggerNoopAsyncSink_NoCopy_1(benchmark::State& state)
{
    auto poller = logpp::AsyncQueuePoller::create();
    auto asyncSink = std::make_shared<logpp::sink::AsyncSink>(poller, std::make_shared<NoopSink>());

    poller->start();

    auto logger = logpp::LoggerFactory::getLogger("BM_BenchLoggerNoopAsyncSink_NoCopy_1", asyncSink);
    uint64_t i = 0;

    for (auto _: state)
    {
        logger->debug("Looping", logpp::data("Iteration", i));
        ++i;
    }

    poller->stop();
}

static void BM_BenchLoggerNoopSink_NoCopy_2(benchmark::State& state)
{
    auto logger = logpp::LoggerFactory::getLogger("BM_BenchLoggerNoopSink_NoCopy_2", std::make_shared<NoopSink>());
    uint64_t i = 0;

    for (auto _: state)
    {
        logger->debug("Looping",
            logpp::data("Int", i),
            logpp::data("Double", double(i))
        );

        ++i;
    }
}

static void BM_BenchLoggerNoopSink_NoCopy_3(benchmark::State& state)
{
    auto logger = logpp::LoggerFactory::getLogger("BM_BenchLoggerNoopSink_NoCopy_3", std::make_shared<NoopSink>());
    uint64_t i = 0;

    std::string name = "BM_BenchLoggerNoopSink3";

    for (auto _: state)
    {
        logger->debug("Looping",
            logpp::data("Int", i),
            logpp::data("Double", double(i)),
            logpp::data("BenchmarkName", name)
        );

        ++i;
    }
}

static void BM_BenchLoggerNoopSink_NoCopy_LargeLogBuffer(benchmark::State& state)
{
    auto logger = logpp::LoggerFactory::getLogger("BM_BenchLoggerNoopSink_NoCopy_LargeLogBuffer", std::make_shared<NoopSink>());

    auto str1 = generateRandomString(10);
    auto str2 = generateRandomString(20);
    auto str3 = generateRandomString(40);
    auto str4 = generateRandomString(512);

    for (auto _: state)
    {
        logger->debug("Looping",
            logpp::data("Str1", str1),
            logpp::data("Str2", str2),
            logpp::data("Str3", str3),
            logpp::data("Str4", str4)
        );
    }
}

BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_1);
BENCHMARK(BM_BenchLoggerNoopSink_Copy_1);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_2);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_3);
BENCHMARK(BM_BenchLoggerNoopAsyncSink_NoCopy_1);
BENCHMARK(BM_BenchLoggerNoopSink_NoCopy_LargeLogBuffer);

BENCHMARK_MAIN();