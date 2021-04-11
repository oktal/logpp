#pragma once

#include "logpp/queue/IAsyncQueuePoller.h"
#include "logpp/queue/ITypedAsyncQueue.h"
#include "logpp/threading/AffinityMask.h"

#include <optional>
#include <thread>
#include <variant>
#include <vector>

namespace logpp
{
    class AsyncQueuePoller : public IAsyncQueuePoller,
                             public std::enable_shared_from_this<AsyncQueuePoller>
    {
    public:
        static constexpr size_t DefaultInternalQueueSize = 16;

        struct Options
        {
            //ThreadHelper::ThreadPriority priority;
            std::optional<threading::AffinityMask> affinity;
            size_t internalQueueSize = DefaultInternalQueueSize;
        };

        static const Options DefaultOptions;

        ~AsyncQueuePoller();

        static std::shared_ptr<AsyncQueuePoller> create(Options options = DefaultOptions);

        void start() override;
        void stop() override;

        void addQueue(std::shared_ptr<IAsyncQueue> queue) override;
        std::future<size_t> removeQueue(std::shared_ptr<IAsyncQueue> queue) override;

    private:
        AsyncQueuePoller(Options options);

        enum class ControlAction {
            Add,
            Remove
        };

        struct StopEntry
        { };
        struct QueueEntry
        {
            std::shared_ptr<IAsyncQueue> queue;
            ControlAction action;

            std::shared_ptr<std::promise<size_t>> handlePromise;
        };

        using Entry = std::variant<StopEntry, QueueEntry>;

        Options m_options;
        std::shared_ptr<ITypedAsyncQueue<Entry>> m_internalQueue;
        std::vector<std::shared_ptr<IAsyncQueue>> m_queues;

        std::atomic<bool> m_running { false };
        std::thread m_thread;

        std::atomic<bool> m_stopSignal { false };

        void handleEntry(StopEntry entry);
        void handleEntry(QueueEntry entry);

        void handleQueue(QueueEntry entry);
        void run(Options options);
    };

}
