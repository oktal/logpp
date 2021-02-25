#pragma once

#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <iostream>

#include "logpp/config/FileWatcher.h"
#include "logpp/queue/IAsyncQueue.h"

namespace logpp
{
    class FileWatcher::Impl : public std::enable_shared_from_this<FileWatcher::Impl>
    {
    public:
        ~Impl()
        {
            stop();
        }

        Impl(std::shared_ptr<IAsyncQueuePoller> queuePoller)
            : m_queuePoller(queuePoller)
        {
            auto fd = ::inotify_init1(IN_NONBLOCK);
            if (fd < 0)
                throw std::runtime_error("Failed to init inotify");

            m_fd = fd;
        }

        void start()
        {
            m_queue = std::make_shared<NotifyQueue>(m_fd, shared_from_this());
            m_queuePoller->addQueue(m_queue);
        }

        void stop()
        {
            if (m_fd > 0)
                ::close(m_fd);

            if (m_queue)
                m_queuePoller->removeQueue(m_queue).get();
        }

        std::optional<FileWatcher::WatchId> addWatch(std::string_view path, FileWatcher::OnEvent onEvent)
        {
            auto wd = ::inotify_add_watch(m_fd, path.data(), IN_MODIFY);
            if (wd < 0)
                return std::nullopt;

            FileWatcher::WatchId watchId(wd);
            WatchEntry entry { watchId, std::string(path), std::move(onEvent) };

            m_entries.insert(std::make_pair(wd, std::move(entry)));
            return watchId;
        }

        bool removeWatch(FileWatcher::WatchId watchId)
        {
            if (m_fd < 0)
                return false;

            auto wd  = static_cast<int>(static_cast<uint64_t>(watchId));
            auto res = inotify_rm_watch(m_fd, wd);
            return res == 0;
        }

        void handleEvents()
        {
            // Some systems cannot read integer variables if they are not
            // properly aligned. On other systems, incorrect alignment may
            // decrease performance. Hence, the buffer used for reading from
            // the inotify file descriptor should have the same alignment as
            // struct inotify_event.
            char buf[4096]
                __attribute__((aligned(__alignof__(struct inotify_event))));
            const struct inotify_event* event;
            ssize_t len;

            for (;;)
            {
                len = ::read(m_fd, buf, sizeof(buf));
                if (len == -1)
                {
                    if (errno == EAGAIN)
                        break;

                    // The read returned with an error. Maybe should we trigger all the registered
                    // callbacks with an error instead of silently stoping ?
                    stop();
                }

                for (char* ptr = buf; ptr < buf + len; ptr += sizeof(inotify_event) + event->len)
                {
                    event = reinterpret_cast<inotify_event*>(ptr);

                    // The watch has been removed and triggered an IN_IGNORED event
                    if (event->mask & IN_IGNORED)
                        m_entries.erase(event->wd);
                    else
                    {
                        invokeCallback(event->wd, event->mask);
                    }
                }
            }
        }

    private:
        class NotifyQueue : public logpp::IAsyncQueue
        {
        public:
            NotifyQueue(int fd, std::shared_ptr<FileWatcher::Impl> watcher)
                : m_fd(fd)
                , m_watcher(std::move(watcher))
            { }

            size_t pollOne() override
            {
                struct pollfd fds[] = {
                    { m_fd, POLLIN }
                };

                auto res = ::poll(&fds[0], 1, 0);
                if (res <= 0)
                    return 0;

                if (fds[0].revents & POLLIN)
                {
                    m_watcher->handleEvents();
                }

                return res;
            }

            size_t poll() override
            {
                return pollOne();
            }

        private:
            int m_fd;

            std::shared_ptr<FileWatcher::Impl> m_watcher;
        };

        struct WatchEntry
        {
            WatchId id;
            std::string path;

            OnEvent callback;

            void invoke()
            {
                callback(path);
            }
        };

        std::shared_ptr<IAsyncQueuePoller> m_queuePoller;
        int m_fd;
        std::shared_ptr<NotifyQueue> m_queue;

        std::unordered_map<int, WatchEntry> m_entries;

        void invokeCallback(int wd, int /*mask*/)
        {
            auto entryIt = m_entries.find(wd);
            if (entryIt != std::end(m_entries))
                entryIt->second.invoke();
        }
    };
}
