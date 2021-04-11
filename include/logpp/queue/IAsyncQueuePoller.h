#pragma once

#include "logpp/queue/IAsyncQueue.h"
#include "logpp/threading/AffinityMask.h"

#include <future>
#include <optional>

namespace logpp
{
    class IAsyncQueuePoller
    {
    public:
        virtual ~IAsyncQueuePoller() = default;

        virtual void start() = 0;
        virtual void stop()  = 0;

        virtual void addQueue(std::shared_ptr<IAsyncQueue> queue)                   = 0;
        virtual std::future<size_t> removeQueue(std::shared_ptr<IAsyncQueue> queue) = 0;
    };
}
