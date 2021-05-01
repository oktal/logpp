#include "logpp/queue/AsyncQueuePoller.h"
#include "logpp/queue/BoundedTypedConcurrentAsyncQueue.h"

#include "SpinWait.h"

#include <deque>

namespace logpp
{

    template <typename Entry>
    class SimpleBlockingQueue : public ITypedAsyncQueue<Entry>
    {
    public:
        using Handler = typename ITypedAsyncQueue<Entry>::Handler;

        size_t poll() override
        {
            size_t totalCount = 0;

            std::scoped_lock<std::mutex> guard { m_mutex };

            for (;;)
            {
                if (m_queue.empty())
                    break;

                auto& entry = m_queue.front();
                handleEntry(entry);
                m_queue.pop_front();
                ++totalCount;
            }

            return totalCount;
        }

        size_t pollOne() override
        {
            std::scoped_lock<std::mutex> guard { m_mutex };
            if (m_queue.empty())
                return 0;

            auto& entry = m_queue.front();
            handleEntry(entry);
            m_queue.pop_front();
            return 1;
        }

        void setHandler(const Handler& handler) override
        {
            m_handler = handler;
        }

        void push(const Entry& entry) override
        {
            m_queue.push_back(entry);
        }

        void push(Entry&& entry) override
        {
            m_queue.push_back(std::move(entry));
        }

    private:
        std::mutex m_mutex;
        std::deque<Entry> m_queue;

        Handler m_handler;

        void handleEntry(const Entry& entry)
        {
            if (m_handler)
                m_handler(entry);
        }
    };

    const AsyncQueuePoller::Options AsyncQueuePoller::DefaultOptions {
        //ThreadHelper::ThreadPriority::LOW,
        std::nullopt,
    };

    std::shared_ptr<AsyncQueuePoller> AsyncQueuePoller::create(Options options)
    {
        return std::shared_ptr<AsyncQueuePoller>(new AsyncQueuePoller(options));
    }

    AsyncQueuePoller::AsyncQueuePoller(Options options)
        : m_options(options)
        , m_internalQueue(std::make_shared<SimpleBlockingQueue<Entry>>())
    { }

    AsyncQueuePoller::~AsyncQueuePoller()
    {
        AsyncQueuePoller::stop();

        if (m_thread.joinable())
            m_thread.join();
    }

    void AsyncQueuePoller::start()
    {
        bool expected = false;
        if (!m_running.compare_exchange_strong(expected, true))
            return;

        m_internalQueue->setHandler([self = shared_from_this()](const Entry& entry) {
            std::visit([self](const auto& e) {
                self->handleEntry(e);
            },
                       entry);
        });

        m_thread = std::thread([=] { this->run(m_options); });
    }

    void AsyncQueuePoller::stop()
    {
        bool expected = true;
        if (!m_running.compare_exchange_strong(expected, false))
            return;

        m_internalQueue->push(StopEntry {});
    }

    void AsyncQueuePoller::addQueue(std::shared_ptr<IAsyncQueue> queue)
    {
        QueueEntry entry { std::move(queue), ControlAction::Add, nullptr };
        m_internalQueue->push(std::move(entry));
    }

    std::future<size_t> AsyncQueuePoller::removeQueue(std::shared_ptr<IAsyncQueue> queue)
    {
        auto handlePromise = std::make_shared<std::promise<size_t>>();
        auto future        = handlePromise->get_future();

        if (!m_running.load(std::memory_order_relaxed))
        {
            handlePromise->set_exception(
                std::make_exception_ptr(std::runtime_error("Poller is not started. Attempting to remove and wait from the future might deadlock.")));
        }
        else
        {
            QueueEntry entry { std::move(queue), ControlAction::Remove, std::move(handlePromise) };
            m_internalQueue->push(std::move(entry));
        }

        return future;
    }

    void AsyncQueuePoller::handleEntry(StopEntry)
    {
        m_stopSignal.store(true, std::memory_order_relaxed);
        ;
    }

    void AsyncQueuePoller::handleEntry(QueueEntry entry)
    {
        handleQueue(std::move(entry));
    }

    void AsyncQueuePoller::handleQueue(QueueEntry entry)
    {
        auto queue  = entry.queue;
        auto action = entry.action;

        auto queueIt = std::find(std::begin(m_queues), std::end(m_queues), queue);
        if (action == ControlAction::Add && queueIt == std::end(m_queues))
        {
            m_queues.push_back(std::move(queue));
        }
        else if (action == ControlAction::Remove)
        {
            size_t count = 0;
            if (queueIt != std::end(m_queues))
            {
                // Let's poll the last entries before removing the queue
                count = queue->poll();
                m_queues.erase(queueIt);
            }
            if (entry.handlePromise)
                entry.handlePromise->set_value(count);
        }
    }

    void AsyncQueuePoller::run(Options options)
    {
        // ThreadHelper::setThreadName("AsyncQueuePoller");
        // ThreadHelper::setThreadPriority(options.priority);
        // if (options.affinity)
        //     ThreadHelper::setThreadAffinity(*options.affinity);

        SpinWait spinWait;

        for (;;)
        {
            size_t totalCount = 0;
            totalCount += m_internalQueue->poll();

            for (auto& queue : m_queues)
            {
                totalCount += queue->poll();
            }

            if (totalCount == 0)
                spinWait.spinOnce();

            if (m_stopSignal.load(std::memory_order_relaxed))
                break;
        }
    }
}
