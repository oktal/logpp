#include "logpp/config/FileWatcher.h"
#include "logpp/queue/AsyncQueuePoller.h"
#include "logpp/core/config.h"

#if defined(LOGPP_PLATFORM_LINUX)
  #include "logpp/config/impl/FileWatcherLinuxImpl.h"
#elif defined(LOGPP_PLATFORM_WINDOWS)
  #include "logpp/config/impl/FileWatcherWindowsImpl.h"
#else
  #error "FileWatcher implementation not available on current platform"
#endif

namespace logpp
{
    FileWatcher::FileWatcher()
        : m_internalPoller(AsyncQueuePoller::create(AsyncQueuePoller::DefaultOptions)) 
        , m_impl(std::make_shared<Impl>(m_internalPoller))
    {}

    FileWatcher::FileWatcher(std::shared_ptr<IAsyncQueuePoller> queuePoller)
        : m_internalPoller(nullptr)
        , m_impl(std::make_shared<Impl>(std::move(queuePoller)))
    {}

    void FileWatcher::start()
    {
        if (m_internalPoller)
            m_internalPoller->start();
        m_impl->start();
    }

    void FileWatcher::stop()
    {
        if (m_internalPoller)
            m_internalPoller->stop();
        m_impl->stop();
    }

    std::optional<FileWatcher::WatchId> FileWatcher::addWatch(std::string_view path, FileWatcher::OnEvent onEvent)
    {
        return m_impl->addWatch(path, std::move(onEvent));
    }

    bool FileWatcher::removeWatch(FileWatcher::WatchId watchId)
    {
        return m_impl->removeWatch(watchId);
    }
}