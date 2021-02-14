#pragma once

#include "logpp/core/config.h"
#include "logpp/queue/IAsyncQueuePoller.h"

#include <functional>
#include <memory>
#include <optional>

namespace logpp
{
    class FileWatcher
    {
    public:
        using OnEvent = std::function<void (std::string_view)>;

        struct WatchId
        {
        public:
            explicit WatchId(uint64_t value)
                : value(value)
            {}

            explicit operator uint64_t() const { return value; }

        private:
            uint64_t value;
        };

        FileWatcher();
        explicit FileWatcher(std::shared_ptr<IAsyncQueuePoller> queuePoller);

        void start();
        void stop();

        std::optional<WatchId> addWatch(std::string_view path, OnEvent onEvent);
        bool removeWatch(WatchId watchId);

    private:
        std::shared_ptr<IAsyncQueuePoller> m_internalPoller;

        class Impl;
        std::shared_ptr<Impl> m_impl;
    };
}