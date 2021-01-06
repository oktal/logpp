#include "logpp/core/LoggerFactory.h"

#include "logpp/queue/AsyncQueuePoller.h"
#include "logpp/sinks/AsyncSink.h"
#include "logpp/sinks/LogFmt.h"

#include <chrono>
#include <iostream>
#include <thread>

struct BidEvent
{
    logpp::Offset<uint64_t> id;
    logpp::Offset<double> price;

    void format(logpp::LogBufferView buffer, logpp::LogWriter& writer) const
    {
        writer.write("id", buffer, id);
        writer.write("price", buffer, price);
    }
};

void doBid(std::shared_ptr<logpp::Logger> logger, uint64_t id, double price)
{
    logger->debug("Submitting bid", [&](logpp::LogBufferBase& buffer, BidEvent& event) {
        event.id = buffer.write(id);
        event.price = buffer.write(price);
    });
}

int main()
{
    auto poller = logpp::AsyncQueuePoller::create();

    auto logFmt = std::make_shared<logpp::sink::LogFmt>(std::cout);
    auto asyncSink = std::make_shared<logpp::sink::AsyncSink>(poller, logFmt);

    auto logger = logpp::LoggerFactory::getLogger("main", asyncSink);

    poller->start();
    asyncSink->start();

    doBid(logger, 0xDEAD, 10.0);

    std::this_thread::sleep_for(std::chrono::seconds(1));
}