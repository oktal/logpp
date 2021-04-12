#include "logpp/queue/AsyncQueuePoller.h"
#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/ColoredConsole.h"

#include <chrono>
#include <iostream>
#include <thread>

void doBid(std::shared_ptr<logpp::Logger> logger, uint64_t id, double price)
{
    logger->debug("Submitting bid",
                  logpp::field("id", id),
                  logpp::field("price", price));
}

int main()
{
    auto poller = logpp::AsyncQueuePoller::create();

    auto logFmt    = std::make_shared<logpp::sink::ColoredOutputConsole>();
    auto asyncSink = std::make_shared<logpp::sink::AsyncSink>(poller, logFmt);

    auto logger = std::make_shared<logpp::Logger>("main", logpp::LogLevel::Debug, asyncSink);

    poller->start();
    asyncSink->start();

    doBid(logger, 0xDEAD, 10.0);

    std::this_thread::sleep_for(std::chrono::seconds(1));
}
