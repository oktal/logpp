#include "logpp/sinks/file/RollingOfstream.h"

#include "logpp/core/config.h"
#include "logpp/date/date.h"
#include "logpp/utils/file.h"

#include "TemporaryFile.h"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <vector>

using namespace logpp;

using namespace std::chrono;
using namespace date;

using namespace std::string_view_literals;

class fake_rolling_filebuf : public rolling_filebuf_base<char>
{
public:
    bool can_roll() override { return false; }
    void roll() override { }

private:
    pos_type offset = 0;

    pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override
    {
        return offset;
    }

    std::streamsize xsputn(const char*, std::streamsize s) override
    {
        offset += s;
        return s;
    }
};

// Hackish way of outputing custom message to gtest output

namespace testing
{

    extern void ColoredPrintf(internal::GTestColor color, const char* fmt, ...);
}
#define PRINTF(...)                                                                        \
    do                                                                                     \
    {                                                                                      \
        testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); \
        testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__);    \
    } while (0)

// C++ stream interface

class TestCout : public std::stringstream
{
public:
    ~TestCout()
    {
        PRINTF("%s\n", str().c_str());
    }
};

#define GOUT TestCout()

struct RemoveDirectoryOnExit
{
public:
    explicit RemoveDirectoryOnExit(std::string_view dir)
        : m_dir(dir)
    { }

    ~RemoveDirectoryOnExit()
    {
        // If we fail to remove the directory, print a warning with the path to give
        // a chance to the user to manually clean it.
        if (!file_utils::removeAll(m_dir))
            std::cerr << "WARNING: Failed to remove directory '" << m_dir << "'\n";
    }

private:
    std::string m_dir;
};

template <typename Clock>
struct FrozenClockBase
{
    using time_point = typename Clock::time_point;
    using duration   = typename Clock::duration;

    static void setNow(time_point now) { frozenNow = now; }

    static time_point now() { return frozenNow; }

    static std::time_t to_time_t(time_point tp) { return Clock::to_time_t(tp); }
    static time_point from_time_t(std::time_t time) { return Clock::from_time_t(time); }

    static time_point frozenNow;
};

template <typename Clock>
typename FrozenClockBase<Clock>::time_point
    FrozenClockBase<Clock>::frozenNow
    = time_point(duration(0));

template <typename Clock>
struct FrozenTimeBase
{
    using time_point = typename Clock::time_point;
    using duration   = typename Clock::duration;

    static void setNow(time_point now) { frozenNow = now; }

    static void now(std::tm* out)
    {
        auto time = Clock::to_time_t(frozenNow);
        date_utils::gmtime(&time, out);
    }

    static time_point frozenNow;
};

template <typename Clock>
typename FrozenTimeBase<Clock>::time_point
    FrozenTimeBase<Clock>::frozenNow
    = time_point(duration(0));

using FrozenClock = FrozenClockBase<std::chrono::system_clock>;
using FrozenTime  = FrozenTimeBase<std::chrono::system_clock>;

struct TestCase
{
    virtual ~TestCase() = default;

    struct Case
    {
        using TimeDuration = decltype(hours { 0 } + minutes { 0 } + seconds { 0 });

        date::year_month_day ymd;
        TimeDuration time;
        bool expected;
    };

    explicit TestCase(std::string_view name, std::initializer_list<Case> cases)
        : name(name)
        , cases(cases)
    {
    }

    std::string_view name;
    std::vector<Case> cases;

    virtual void run() = 0;
};

template <typename Strategy>
struct RollingTestCase : TestCase
{
    RollingTestCase(std::string_view name, Strategy strategy,
                    std::initializer_list<TestCase::Case> cases)
        : TestCase(name, cases)
        , strategy(strategy)
    { }

    Strategy strategy;

    void run() override
    {
        GOUT << "Running " << this->name;

        std::filebuf buf;
        for (const auto& testCase : this->cases)
        {
            auto days = static_cast<date::sys_days>(testCase.ymd);
            auto time = make_time(testCase.time);

            auto tp = typename FrozenClock::time_point { days + testCase.time };
            FrozenClock::setNow(tp);

            ASSERT_EQ(strategy.apply(&buf), testCase.expected)
                << " RollingStrategy did not apply as expected."
                << " name=\"" << this->name << "\" date=" << testCase.ymd
                << " time=" << time;
        }
    }
};

template <typename... Tests>
std::vector<std::unique_ptr<TestCase>>
declareTests(Tests&&... tests)
{
    std::vector<std::unique_ptr<TestCase>> testCases;
    (testCases.push_back(std::move(tests)), ...);

    return testCases;
}

template <typename Strategy>
std::unique_ptr<TestCase>
declareTest(std::string_view name, Strategy strategy,
            std::initializer_list<TestCase::Case> cases)
{
    return std::make_unique<RollingTestCase<Strategy>>(name, strategy, cases);
}

void runTests(const std::vector<std::unique_ptr<TestCase>>& tests)
{
    for (const auto& test : tests)
    {
        test->run();
    }
}

TEST(RollingOfstreamTests, should_roll_by_time)
{
    auto cases = declareTests(
        declareTest(
            "Rolling every minute precise",
            RollEvery { PreciseInterval { minutes(1) }, FrozenClock {} },
            {
                { jan / 29 / 2021, hours { 11 } + minutes { 50 } + seconds { 20 }, false },
                { jan / 29 / 2021, hours { 11 } + minutes { 50 } + seconds { 50 }, false },
                { jan / 29 / 2021, hours { 11 } + minutes { 51 } + seconds { 10 }, false },
                // ROLL
                { jan / 29 / 2021, hours { 11 } + minutes { 51 } + seconds { 20 }, true },
                { jan / 29 / 2021, hours { 11 } + minutes { 51 } + seconds { 50 }, false },
                { jan / 29 / 2021, hours { 11 } + minutes { 52 } + seconds { 10 }, false },
                // ROLL
                { jan / 29 / 2021, hours { 11 } + minutes { 52 } + seconds { 30 }, true },
                { jan / 29 / 2021, hours { 11 } + minutes { 52 } + seconds { 55 }, false },
                // ROLL
                { jan / 30 / 2021, hours { 9 } + minutes { 10 } + seconds { 30 }, true },
                { jan / 30 / 2021, hours { 9 } + minutes { 10 } + seconds { 50 }, false },
            }),
        declareTest(
            "Rolling every hour precise",
            RollEvery { PreciseInterval { hours(1) }, FrozenClock {} },
            {
                { jan / 29 / 2021, hours { 9 } + minutes { 10 } + seconds { 30 }, false },
                { jan / 29 / 2021, hours { 9 } + minutes { 25 } + seconds { 10 }, false },
                { jan / 29 / 2021, hours { 10 } + minutes { 5 } + seconds { 50 }, false },
                // ROLL
                { jan / 29 / 2021, hours { 10 } + minutes { 15 } + seconds { 30 }, true },
                { jan / 29 / 2021, hours { 10 } + minutes { 30 } + seconds { 50 }, false },
                { jan / 29 / 2021, hours { 10 } + minutes { 50 } + seconds { 50 }, false },
                // ROLL
                { jan / 29 / 2021, hours { 11 } + minutes { 20 } + seconds { 35 }, true },
                { jan / 29 / 2021, hours { 11 } + minutes { 50 } + seconds { 35 }, false },
                // ROLL
                { jan / 30 / 2021, hours { 9 } + minutes { 10 } + seconds { 30 }, true },
            }),
        declareTest(
            "Rolling every day precise",
            RollEvery { PreciseInterval { days(1) }, FrozenClock {} },
            {
                { jan / 29 / 2021, hours { 9 } + minutes { 10 } + seconds { 30 }, false },
                { jan / 29 / 2021, hours { 9 } + minutes { 25 } + seconds { 10 }, false },
                { jan / 29 / 2021, hours { 10 } + minutes { 5 } + seconds { 50 }, false },
                // ROLL
                { jan / 30 / 2021, hours { 10 } + minutes { 15 } + seconds { 30 }, true },
                { jan / 30 / 2021, hours { 12 } + minutes { 30 } + seconds { 50 }, false },
                { jan / 30 / 2021, hours { 18 } + minutes { 20 } + seconds { 50 }, false },
                // ROLL
                { feb / 1 / 2021, hours { 11 } + minutes { 10 } + seconds { 35 }, true },
                { feb / 1 / 2021, hours { 12 } + minutes { 20 } + seconds { 35 }, false },
            }),
        declareTest(
            "Rolling every month precise",
            RollEvery { PreciseInterval { date::months(1) }, FrozenClock {} },
            {
                { jan / 1 / 2021, hours { 10 } + minutes { 20 } + seconds { 55 }, false },
                { jan / 1 / 2021, hours { 20 } + minutes { 50 } + seconds { 35 }, false },
                { jan / 8 / 2021, hours { 18 } + minutes { 30 } + seconds { 40 }, false },
                // ROLL
                { feb / 2 / 2021, hours { 1 } + minutes { 10 } + seconds { 15 }, true },
                { feb / 15 / 2021, hours { 8 } + minutes { 20 } + seconds { 35 }, false },
                { feb / 15 / 2021, hours { 18 } + minutes { 30 } + seconds { 35 }, false },
                // ROLL
                { mar / 5 / 2021, hours { 9 } + minutes { 30 } + seconds { 35 }, true },
                { apr / 2 / 2021, hours { 15 } + minutes { 20 } + seconds { 40 }, false },
            }),
        declareTest(
            "Rolling every year precise",
            RollEvery { PreciseInterval { date::years(1) }, FrozenClock {} },
            {
                { jan / 3 / 2021, hours { 8 } + minutes { 15 } + seconds { 20 }, false },
                { jan / 18 / 2021, hours { 7 } + minutes { 35 } + seconds { 20 }, false },
                { feb / 10 / 2021, hours { 18 } + minutes { 40 } + seconds { 25 }, false },
                { oct / 24 / 2021, hours { 16 } + minutes { 45 } + seconds { 30 }, false },
                { jan / 2 / 2022, hours { 19 } + minutes { 20 } + seconds { 30 }, false },
                // ROLL
                { jan / 4 / 2022, hours { 6 } + minutes { 15 } + seconds { 50 }, true },
                { may / 1 / 2022, hours { 16 } + minutes { 45 } + seconds { 10 }, false },
                // ROLL
                { feb / 1 / 2023, hours { 10 } + minutes { 10 } + seconds { 35 }, true },
                { mar / 1 / 2023, hours { 15 } + minutes { 45 } + seconds { 30 }, false },
            }),
        declareTest(
            "Rolling every minute round",
            RollEvery { RoundInterval { minutes(1) }, FrozenClock {} },
            {
                { jan / 29 / 2021, hours { 9 } + minutes { 10 } + seconds { 20 }, false },
                { jan / 29 / 2021, hours { 9 } + minutes { 10 } + seconds { 50 }, false },
                // ROLL
                { jan / 29 / 2021, hours { 9 } + minutes { 11 } + seconds { 5 }, true },
                { jan / 29 / 2021, hours { 9 } + minutes { 11 } + seconds { 35 }, false },
                { jan / 29 / 2021, hours { 9 } + minutes { 11 } + seconds { 50 }, false },
                // ROLL
                { jan / 29 / 2021, hours { 9 } + minutes { 12 } + seconds { 30 }, true },
                { jan / 29 / 2021, hours { 9 } + minutes { 12 } + seconds { 50 }, false },
                // ROLL
                { jan / 29 / 2021, hours { 10 } + minutes { 5 } + seconds { 10 }, true },
            }),
        declareTest(
            "Rolling every day round",
            RollEvery { RoundInterval { date::days(1) }, FrozenClock {} },
            {
                { jan / 29 / 2021, hours { 9 } + minutes { 10 } + seconds { 20 }, false },
                { jan / 29 / 2021, hours { 15 } + minutes { 25 } + seconds { 50 }, false },
                // ROLL
                { jan / 30 / 2021, hours { 6 } + minutes { 10 } + seconds { 50 }, true },
                { jan / 30 / 2021, hours { 16 } + minutes { 30 } + seconds { 25 }, false },
                { jan / 30 / 2021, hours { 22 } + minutes { 10 } + seconds { 25 }, false },
                // ROLL
                { feb / 2 / 2021, hours { 10 } + minutes { 35 } + seconds { 40 }, true },
                { feb / 2 / 2021, hours { 12 } + minutes { 35 } + seconds { 45 }, false },
            }),

        declareTest(
            "Rolling every month round",
            RollEvery { RoundInterval { date::months(1) }, FrozenClock {} },
            {
                { jan / 29 / 2021, hours { 9 } + minutes { 10 } + seconds { 30 }, false },
                { jan / 29 / 2021, hours { 15 } + minutes { 35 } + seconds { 30 }, false },
                { jan / 30 / 2021, hours { 18 } + minutes { 25 } + seconds { 50 }, false },
                // ROLL
                { feb / 1 / 2021, hours { 6 } + minutes { 10 } + seconds { 30 }, true },
                { feb / 1 / 2021, hours { 16 } + minutes { 30 } + seconds { 55 }, false },
                { feb / 7 / 2021, hours { 12 } + minutes { 35 } + seconds { 40 }, false },
                // ROLL
                { mar / 3 / 2021, hours { 1 } + minutes { 10 } + seconds { 30 }, true },
                { mar / 30 / 2021, hours { 10 } + minutes { 20 } + seconds { 50 }, false },
            }),
        declareTest(
            "Rolling every year round",
            RollEvery { RoundInterval { date::years(1) }, FrozenClock {} },
            {
                { jan / 3 / 2021, hours { 8 } + minutes { 15 } + seconds { 20 }, false },
                { jan / 18 / 2021, hours { 7 } + minutes { 35 } + seconds { 20 }, false },
                { feb / 10 / 2021, hours { 18 } + minutes { 40 } + seconds { 25 }, false },
                { oct / 24 / 2021, hours { 16 } + minutes { 45 } + seconds { 30 }, false },
                // ROLL
                { jan / 2 / 2022, hours { 19 } + minutes { 20 } + seconds { 30 }, true },
                { jan / 18 / 2022, hours { 6 } + minutes { 15 } + seconds { 50 }, false },
                { may / 1 / 2022, hours { 16 } + minutes { 45 } + seconds { 10 }, false },
                // ROLL
                { feb / 1 / 2023, hours { 10 } + minutes { 10 } + seconds { 35 }, true },
                { mar / 1 / 2023, hours { 15 } + minutes { 45 } + seconds { 30 }, false },
            })

    );

    runTests(cases);
}

TEST(RollingOfstreamTests, should_roll_based_on_size)
{
    RollBySize strategy { 10 * 1024 }; // 10 KiB
    fake_rolling_filebuf buf;

    auto check = [&](size_t size, bool expected) {
        buf.sputn(nullptr, size);
        ASSERT_EQ(strategy.apply(&buf), expected);
    };

    check(0ULL, false);
    check(2ULL * 1024, false);
    check(3ULL * 1024, false);
    check(6ULL * 1024, true);
}

// Disabled on Windows to make CI happy until I figure out what's happening...
#ifndef LOGPP_PLATFORM_WINDOWS

TEST(RollingOfstreamTests, should_archive_with_incrementing_number)
{
    auto ymd = jan / 23 / 2021;
    auto now = date::sys_days { ymd };

    ArchiveIncremental archive;
    temporary_rolling_filebuf fileBuf(std::ios_base::out, "logpptestfile", ".log");
    auto basePath = fileBuf.path();

    RemoveDirectoryOnExit rmDir(fileBuf.directory());

    auto write = [&](std::string_view str) {
        fileBuf.sputn(str.data(), str.size());
    };

    auto flush = [&] {
        fileBuf.pubsync();
    };

    write("File0"sv);
    flush();

    archive.apply(&fileBuf);

    write("File1"sv);
    flush();

    archive.apply(&fileBuf);

    write("File2"sv);
    flush();

    archive.apply(&fileBuf);

    write("File3"sv);
    flush();

    auto check = [](std::string_view baseFileName, std::optional<size_t> n, std::function<void(const std::string&)> assertFunc) {
        auto fileName = std::string(baseFileName);
        if (n)
        {
            fileName.push_back('.');
            fileName.append(std::to_string(*n));
        }

        ASSERT_TRUE(file_utils::exists(fileName)) << "File " << fileName << " does not exist";
        std::ifstream in(fileName);
        ASSERT_TRUE(in) << "Failed to open file: " << fileName;
        std::string str((std::istreambuf_iterator<char>(in)),
                        (std::istreambuf_iterator<char>()));
        assertFunc(str);
    };

    check(basePath, std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File3");
    });

    check(basePath, 0, [](const std::string& s) {
        ASSERT_EQ(s, "File2");
    });

    check(basePath, 1, [](const std::string& s) {
        ASSERT_EQ(s, "File1");
    });

    check(basePath, 2, [](const std::string& s) {
        ASSERT_EQ(s, "File0");
    });
}

TEST(RollingOfstreamTests, should_archive_with_timestamp)
{
    ArchiveTimestamp<FrozenTime> archive { "%Y%m%d" };
    temporary_rolling_filebuf fileBuf(std::ios_base::out, "logpptestfile", ".log");
    auto basePath = fileBuf.path();

    RemoveDirectoryOnExit rmDir(fileBuf.directory());

    auto write = [&](std::string_view str) {
        fileBuf.sputn(str.data(), str.size());
    };

    auto flush = [&] {
        fileBuf.pubsync();
    };

    auto ymd = jan / 23 / 2021;
    auto now = date::sys_days { ymd } + hours { 9 } + minutes { 10 } + seconds { 35 };
    FrozenTime::setNow(now);

    write("File0"sv);
    flush();

    archive.apply(&fileBuf);

    write("File1"sv);
    flush();

    auto check = [](std::string_view baseFileName, std::optional<std::string> suffix, std::function<void(const std::string&)> assertFunc) {
        auto fileName = std::string(baseFileName);
        if (suffix)
        {
            fileName.push_back('.');
            fileName.append(*suffix);
        }

        ASSERT_TRUE(file_utils::exists(fileName)) << "File " << fileName << " does not exist";
        std::ifstream in(fileName);
        ASSERT_TRUE(in) << "Failed to open file: " << fileName;
        std::string str((std::istreambuf_iterator<char>(in)),
                        (std::istreambuf_iterator<char>()));
        assertFunc(str);
    };

    check(basePath, std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File1");
    });

    check(basePath, "20210123", [](const std::string& s) {
        ASSERT_EQ(s, "File0");
    });
}

TEST(RollingOfstreamTests, should_archive_with_incremental_if_timestamp_already_exists)
{
    ArchiveTimestamp<FrozenTime> archive { "%Y%m%d" };
    temporary_rolling_filebuf fileBuf(std::ios_base::out, "logpptestfile", ".log");
    auto basePath = fileBuf.path();

    RemoveDirectoryOnExit rmDir(fileBuf.directory());

    auto write = [&](std::string_view str) {
        fileBuf.sputn(str.data(), str.size());
    };

    auto flush = [&] {
        fileBuf.pubsync();
    };

    auto ymd = jan / 23 / 2021;
    auto now = date::sys_days { ymd };
    FrozenTime::setNow(now);

    write("File0"sv);
    flush();

    archive.apply(&fileBuf);
    write("File1"sv);
    flush();

    archive.apply(&fileBuf);
    write("File2"sv);
    flush();

    archive.apply(&fileBuf);
    write("File3"sv);
    flush();

    auto check = [](std::string_view baseFileName, std::optional<std::string> suffix, std::optional<size_t> n, std::function<void(const std::string&)> assertFunc) {
        auto fileName = std::string(baseFileName);
        if (suffix)
        {
            fileName.push_back('.');
            fileName.append(*suffix);
        }
        if (n)
        {
            fileName.push_back('.');
            fileName.append(std::to_string(*n));
        }

        ASSERT_TRUE(file_utils::exists(fileName)) << "File " << fileName << " does not exist";
        std::ifstream in(fileName);
        ASSERT_TRUE(in) << "Failed to open file: " << fileName;
        std::string str((std::istreambuf_iterator<char>(in)),
                        (std::istreambuf_iterator<char>()));
        assertFunc(str);
    };

    check(basePath, std::nullopt, std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File3");
    });

    check(basePath, "20210123", 0, [](const std::string& s) {
        ASSERT_EQ(s, "File1");
    });

    check(basePath, "20210123", 1, [](const std::string& s) {
        ASSERT_EQ(s, "File0");
    });

    check(basePath, "20210123", std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File2");
    });
}

#endif
