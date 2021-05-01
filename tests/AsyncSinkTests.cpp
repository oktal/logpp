#include "gtest/gtest.h"

#include "logpp/core/Logger.h"

#include "logpp/queue/AsyncQueuePoller.h"

#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/Sink.h"

using namespace logpp;

class MemorySink : public sink::Sink
{
public:
    struct Entry
    {
        std::string_view name;
        LogLevel level;
        EventLogBuffer buffer;
    };

    void activateOptions(const sink::Options&) override { }

    void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override
    {
        std::scoped_lock guard { m_mutex };
        m_entries.push_back(Entry { name, level, buffer });
        m_cv.notify_all();
    }

    template <typename Duration>
    std::vector<Entry> waitForEntries(size_t count, Duration timeout)
    {
        std::unique_lock lock { m_mutex };
        m_cv.wait_for(lock, timeout, [&] {
            return m_entries.size() >= count;
        });

        std::vector<Entry> entries;
        auto first = std::begin(m_entries);
        auto last  = first;
        std::advance(last, count);

        std::copy(first, last, std::back_inserter(entries));
        return entries;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::vector<Entry> m_entries;
};

struct AsyncSinkTest : public ::testing::Test
{
    void SetUp() override
    {
        poller     = AsyncQueuePoller::create();
        memorySink = std::make_shared<MemorySink>();
        asyncSink  = std::make_shared<sink::AsyncSink>(poller, memorySink);

        poller->start();
        asyncSink->start();
    }

    template <typename Duration>
    std::vector<MemorySink::Entry> waitForEntries(size_t count, Duration timeout)
    {
        return memorySink->waitForEntries(count, timeout);
    }

    std::shared_ptr<IAsyncQueuePoller> poller;
    std::shared_ptr<sink::AsyncSink> asyncSink;
    std::shared_ptr<MemorySink> memorySink;
};

TEST_F(AsyncSinkTest, should_sink)
{
    static constexpr size_t Count = 1'000'000;

    auto logger = std::make_shared<Logger>("AsyncSinkTest", LogLevel::Debug, asyncSink);
    for (size_t i = 0; i < Count; ++i)
    {
        logger->info(logpp::format("Test message {}", i), logpp::field("index", i));
    }

    auto entries = waitForEntries(Count, std::chrono::milliseconds(500));
    ASSERT_EQ(entries.size(), Count);
}
