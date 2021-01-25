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

TEST(RollingStrategyTests, should_roll_file_based_on_date)
{
    DateRollingStrategy strategy(RollingInterval::Minute);

    auto time = make_time(hours{11} +  minutes{50} + seconds{10});
    auto tp = TimePoint{seconds(time)};

    FakeFile file;

    auto check = [&](TimePoint tp, bool expected)
    {
        ASSERT_EQ(strategy.apply(tp, &file), expected);
    };

    check(tp, false);
    check(tp + seconds{30}, false);

    check(tp + minutes{1} + seconds{10}, true);
    check(tp + minutes{1} + seconds{50}, false);

    check(tp + minutes{2}, true);
}

TEST(RollingStrategyTests, should_roll_file_based_on_size)
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