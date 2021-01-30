#include "logpp/sinks/file/RollingStrategy.h"

#include "logpp/date/date.h"
#include "logpp/utils/date.h"

#include "gtest/gtest.h"

using namespace logpp;
using namespace logpp::sink;

using namespace date;
using namespace std::chrono;

class FakeFile : public File
{
public:
    size_t write(const char*, size_t size) override
    {
        m_size += size;
        return size;
    }

    size_t size() const override
    {
        return m_size;
    }
private:
    size_t m_size = 0ULL;
};

TEST(RollingStrategyTests, should_roll_file_date_strategy_round)
{
    using TimeDuration = decltype(hours{0} + minutes{0} + seconds{0});

    struct TestCase
    {
        struct TimeCase
        {
            date::year_month_day date;
            TimeDuration time;
            bool expected;
        };

        std::string_view name;
        RollingInterval interval;
        RollingKind kind;

        std::vector<TimeCase> timeCases;
    } testCases[] = {
        {
            "Rolling every minute precise",
            RollingInterval::Minute,
            RollingKind::Precise,
            {
                { jan/29/2021, hours{11} + minutes{50} + seconds{20}, false },
                { jan/29/2021, hours{11} + minutes{50} + seconds{50}, false },
                { jan/29/2021, hours{11} + minutes{51} + seconds{10}, false },
                // ROLL
                { jan/29/2021, hours{11} + minutes{51} + seconds{20}, true },
                { jan/29/2021, hours{11} + minutes{51} + seconds{50}, false },
                { jan/29/2021, hours{11} + minutes{52} + seconds{10}, false },
                // ROLL
                { jan/29/2021, hours{11} + minutes{52} + seconds{30}, true },
                { jan/29/2021, hours{11} + minutes{52} + seconds{55}, false },
                // ROLL
                { jan/30/2021, hours{9} + minutes{10} + seconds{30}, true },
                { jan/30/2021, hours{9} + minutes{10} + seconds{50}, false },
            }
        },
        {
            "Rolling every hour precise",
            RollingInterval::Hour,
            RollingKind::Precise,
            {
                { jan/29/2021, hours{9} + minutes{10} + seconds{30}, false },
                { jan/29/2021, hours{9} + minutes{25} + seconds{10}, false },
                { jan/29/2021, hours{10} + minutes{5} + seconds{50}, false },
                // ROLL
                { jan/29/2021, hours{10} + minutes{15} + seconds{30}, true },
                { jan/29/2021, hours{10} + minutes{30} + seconds{50}, false },
                { jan/29/2021, hours{10} + minutes{50} + seconds{50}, false },
                // ROLL
                { jan/29/2021, hours{11} + minutes{20} + seconds{35}, true },
                { jan/29/2021, hours{11} + minutes{50} + seconds{35}, false },
                // ROLL
                { jan/30/2021, hours{9} + minutes{10} + seconds{30}, true },
            }
        },
        {
            "Rolling every day precise",
            RollingInterval::Day,
            RollingKind::Precise,
            {
                { jan/29/2021, hours{9} + minutes{10} + seconds{30}, false },
                { jan/29/2021, hours{9} + minutes{25} + seconds{10}, false },
                { jan/29/2021, hours{10} + minutes{5} + seconds{50}, false },
                // ROLL
                { jan/30/2021, hours{10} + minutes{15} + seconds{30}, true },
                { jan/30/2021, hours{12} + minutes{30} + seconds{50}, false },
                { jan/30/2021, hours{18} + minutes{20} + seconds{50}, false },
                // ROLL
                { feb/1/2021, hours{11} + minutes{10} + seconds{35}, true },
                { feb/1/2021, hours{12} + minutes{20} + seconds{35}, false },
            }
        },
        {
            "Rolling every month precise",
            RollingInterval::Month,
            RollingKind::Precise,
            {
                { jan/1/2021, hours{10} + minutes{20} + seconds{55}, false },
                { jan/1/2021, hours{20} + minutes{50} + seconds{35}, false },
                { jan/8/2021, hours{18} + minutes{30} + seconds{40}, false },
                // ROLL
                { feb/2/2021, hours{1} + minutes{10} + seconds{15}, true },
                { feb/15/2021, hours{8} + minutes{20} + seconds{35}, false },
                { feb/15/2021, hours{18} + minutes{30} + seconds{35}, false },
                // ROLL
                { mar/5/2021, hours{9} + minutes{30} + seconds{35}, true },
                { apr/2/2021, hours{15} + minutes{20} + seconds{40}, false },
            }
        },
        {
            "Rolling every year precise",
            RollingInterval::Year,
            RollingKind::Precise,
            {
                { jan/3/2021, hours{8} + minutes{15} + seconds{20}, false },
                { jan/18/2021, hours{7} + minutes{35} + seconds{20}, false },
                { feb/10/2021, hours{18} + minutes{40} + seconds{25}, false },
                { oct/24/2021, hours{16} + minutes{45} + seconds{30}, false },
                { jan/2/2022, hours{19} + minutes{20} + seconds{30}, false },
                // ROLL
                { jan/4/2022, hours{6} + minutes{15} + seconds{50}, true },
                { may/1/2022, hours{16} + minutes{45} + seconds{10}, false },
                // ROLL
                { feb/1/2023, hours{10} + minutes{10} + seconds{35}, true },
                { mar/1/2023, hours{15} + minutes{45} + seconds{30}, false },
            }
        },
        {
            "Infinite rolling precise",
            RollingInterval::Infinite,
            RollingKind::Precise,
            {
                { jan/3/2021, hours{8} + minutes{15} + seconds{20}, false },
                { jan/18/2021, hours{7} + minutes{35} + seconds{20}, false },
                { feb/10/2021, hours{18} + minutes{40} + seconds{25}, false },
                { oct/24/2021, hours{16} + minutes{45} + seconds{30}, false },
                { jan/2/2022, hours{19} + minutes{20} + seconds{30}, false },
                { jan/4/2022, hours{6} + minutes{15} + seconds{50}, false },
                { may/1/2022, hours{16} + minutes{45} + seconds{10}, false },
                { feb/1/2023, hours{10} + minutes{10} + seconds{35}, false },
                { mar/1/2023, hours{15} + minutes{45} + seconds{30}, false },
            }
        },
        {
            "Rolling every minute round",
            RollingInterval::Minute,
            RollingKind::Round,
            {
                { jan/29/2021, hours{9} + minutes{10} + seconds{20}, false },
                { jan/29/2021, hours{9} + minutes{10} + seconds{50}, false },
                // ROLL
                { jan/29/2021, hours{9} + minutes{11} + seconds{5}, true },
                { jan/29/2021, hours{9} + minutes{11} + seconds{35}, false },
                { jan/29/2021, hours{9} + minutes{11} + seconds{50}, false },
                // ROLL
                { jan/29/2021, hours{9} + minutes{12} + seconds{30}, true },
                { jan/29/2021, hours{9} + minutes{12} + seconds{50}, false },
                // ROLL
                { jan/29/2021, hours{10} + minutes{5} + seconds{10}, true },
            }
        },
        {
           "Rolling every day round",
           RollingInterval::Day,
           RollingKind::Round,
           {
               { jan/29/2021, hours{9} + minutes{10} + seconds{20}, false },
               { jan/29/2021, hours{15} + minutes{25} + seconds{50}, false },
               // ROLL
               { jan/30/2021, hours{6} + minutes{10} + seconds{50}, true },
               { jan/30/2021, hours{16} + minutes{30} + seconds{25}, false },
               { jan/30/2021, hours{22} + minutes{10} + seconds{25}, false },
               // ROLL
               { feb/2/2021, hours{10} + minutes{35} + seconds{40}, true },
               { feb/2/2021, hours{12} + minutes{35} + seconds{45}, false },
           }
        },
        {
            "Rolling every month round",
            RollingInterval::Month,
            RollingKind::Round,
            {
                { jan/29/2021, hours{9} + minutes{10} + seconds{30}, false },
                { jan/29/2021, hours{15} + minutes{35} + seconds{30}, false },
                { jan/30/2021, hours{18} + minutes{25} + seconds{50}, false },
                // ROLL
                { feb/1/2021, hours{6} + minutes{10} + seconds{30}, true },
                { feb/1/2021, hours{16} + minutes{30} + seconds{55}, false },
                { feb/7/2021, hours{12} + minutes{35} + seconds{40}, false },
                // ROLL
                { mar/3/2021, hours{1} + minutes{10} + seconds{30}, true },
                { mar/30/2021, hours{10} + minutes{20} + seconds{50}, false },
            }
        },
        {
            "Rolling every year round",
            RollingInterval::Year,
            RollingKind::Round,
            {
                { jan/3/2021, hours{8} + minutes{15} + seconds{20}, false },
                { jan/18/2021, hours{7} + minutes{35} + seconds{20}, false },
                { feb/10/2021, hours{18} + minutes{40} + seconds{25}, false },
                { oct/24/2021, hours{16} + minutes{45} + seconds{30}, false },
                // ROLL
                { jan/2/2022, hours{19} + minutes{20} + seconds{30}, true },
                { jan/18/2022, hours{6} + minutes{15} + seconds{50}, false },
                { may/1/2022, hours{16} + minutes{45} + seconds{10}, false },
                // ROLL
                { feb/1/2023, hours{10} + minutes{10} + seconds{35}, true },
                { mar/1/2023, hours{15} + minutes{45} + seconds{30}, false },
            }
        },
    };

    for (const auto& test: testCases)
    {
        DateRollingStrategy strategy(test.interval, test.kind);
        FakeFile file;

        for (const auto& timeCase: test.timeCases)
        {
            auto time = make_time(timeCase.time);
            auto days = static_cast<date::sys_days>(timeCase.date);
            ASSERT_EQ(strategy.apply(TimePoint{days + timeCase.time}, &file), timeCase.expected)
                << " RollingStrategy did not apply as expected."
                << " Name=\"" << test.name << "\" RollingInterval=" << toString(test.interval) << " RollingKind=" << toString(test.kind)
                << " Date=" << timeCase.date << " Time=" << time;
        }
    }
}

TEST(RollingStrategyTests, should_roll_file_size_strategy)
{
    SizeRollingStrategy strategy(10 * 1024); // 10 KiB
    FakeFile file;

    auto check = [&](size_t size, bool expected)
    {
        file.write(nullptr, size);
        ASSERT_EQ(strategy.apply(TimePoint{}, &file), expected);
    };

    check(0ULL, false);
    check(2ULL * 1024, false);
    check(3ULL * 1024, false);
    check(6ULL * 1024, true);
}