#pragma once

#include "logpp/config/FileWatcher.h"

namespace logpp
{
    class FileWatcher::Impl
    {
    public:
        explicit Impl(std::shared_ptr<IAsyncQueuePoller>) { }

        void start() { }
        void stop() { }

        std::optional<FileWatcher::WatchId> addWatch(std::string_view,
                                                     FileWatcher::OnEvent)
        {
            return std::nullopt;
        }

        bool removeWatch(FileWatcher::WatchId) { return false; }
    };
} // namespace logpp
