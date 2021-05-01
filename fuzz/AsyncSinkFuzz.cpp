#include "FuzzStream.h"

#include "logpp/queue/AsyncQueuePoller.h"

#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/Sink.h"

#include <deque>
#include <stddef.h>
#include <stdint.h>

class NoopSink : public logpp::sink::Sink
{
public:
    void activateOptions(const logpp::sink::Options&) override
    { }

    void sink(std::string_view, logpp::LogLevel, const logpp::EventLogBuffer&) override
    { }
};

namespace
{
    std::shared_ptr<logpp::IAsyncQueuePoller> poller;
    std::shared_ptr<logpp::sink::AsyncSink> sink;

    bool Initialize()
    {
        poller = logpp::AsyncQueuePoller::create();
        sink   = std::make_shared<logpp::sink::AsyncSink>(poller, std::make_shared<NoopSink>());
        poller->start();
        sink->start();
        return true;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    static bool Initialized = Initialize();

    logpp::EventLogBuffer buffer;
    logpp::fuzz::FuzzStream stream(data, size);
    stream.fill(buffer);

    sink->sink("LLVMFuzzerTestOneInput", logpp::LogLevel::Debug, buffer);

    return 0;
}
