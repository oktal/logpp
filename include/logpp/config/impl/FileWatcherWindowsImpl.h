
#pragma once

#include "logpp/config/FileWatcher.h"
#include "logpp/utils/file.h"

#define NOMINMAX
#include <windows.h>
#include <shlwapi.h>

#include <chrono>
#include <iostream>
#include <unordered_map>
#include <variant>

namespace logpp
{
    class FileWatcher::Impl : public std::enable_shared_from_this<FileWatcher::Impl>
    {
    public:
        ~Impl()
        {
        }

        explicit Impl(std::shared_ptr<IAsyncQueuePoller> poller)
	        : m_poller(std::move(poller))
            , m_queue(std::make_shared<WatchQueue>())
        {
        }

        void start()
        {
            m_poller->addQueue(m_queue);
        }

        void stop()
        {
            m_poller->removeQueue(m_queue).wait();
        }

        std::optional<FileWatcher::WatchId> addWatch(std::string_view path, FileWatcher::OnEvent onEvent)
        {
            auto future = m_queue->pushActionAsync(WatchQueue::AddWatch{
                std::string(path),
                std::move(onEvent)
			});

            try
            {
                return future.get();
            }
            catch (const std::exception&)
            {
                return std::nullopt;
            }

        }

        bool removeWatch(FileWatcher::WatchId watchId)
        {
            auto future = m_queue->pushActionAsync(WatchQueue::RemoveWatch{ watchId });

            try
            {
                return future.get();
            }
            catch (const std::exception&)
            {
                return false;
            }
        }

    private:
        class WatchQueue : public logpp::IAsyncQueue
        {
        public:
            template<typename T>
            struct AsyncAction
            {
                std::promise<T> result;

                void setValue(T value)
                {
                    result.set_value(std::move(value));
                }

            	template<typename Exception>
                void setException(Exception exc)
                {
                    result.set_exception(std::make_exception_ptr(exc));
                }
            };

            struct AddWatch : AsyncAction<FileWatcher::WatchId>
            {
                AddWatch(std::string path, FileWatcher::OnEvent onEvent)
                    : path(std::move(path))
                    , onEvent(std::move(onEvent))
                {}

                std::string path;
                FileWatcher::OnEvent onEvent;
            };

            struct RemoveWatch : AsyncAction<bool>
            {
                explicit RemoveWatch(FileWatcher::WatchId watchId)
                    : watchId(watchId)
                {}

                FileWatcher::WatchId watchId;
            };

            using Action = std::variant<AddWatch, RemoveWatch>;

            size_t pollOne() override
            {
                size_t count = 0;

                std::vector<Action> pendingActions;
                {
                    std::lock_guard<std::mutex> guard { m_pendingActionsLock };
                    pendingActions = std::move(m_pendingActions);
                }

                count += pendingActions.size();
                for (auto& pendingAction: pendingActions)
                {
                    std::visit([&](auto& action) { handleAction(action); }, pendingAction);
                }

                pollDirectoryChanges();
                count += pollPendingNotifications();
                return count;
            }

            size_t poll() override
            {
                return pollOne();
            }

            template<typename TAction>
            auto pushActionAsync(TAction&& action)
            {
                auto fut = action.result.get_future();
                m_pendingActions.push_back(std::move(action));
                return fut;
            }

        private: 
			struct PathEntry
			{
				uint64_t watchId;
				std::string path;

                FileWatcher::OnEvent onEvent;
			};
            	
            struct DirectoryEntry
            {
                std::string name;
                HANDLE handle;
                std::vector<PathEntry> pathEntries;
            };

            // ReadDirectoryChangesW is an absolute nighmare to work with as it will trigger double notifications, see, e.g
            // https://stackoverflow.com/questions/14036449/c-winapi-readdirectorychangesw-receiving-double-notifications
            // To dedup notifications, we do not trigger an event right away but instead wait for a short period of time.
            // If, during that period of time (1ms), a duplicated notification is received, we just discard it.
            static constexpr auto NotificationDelay{std::chrono::milliseconds(1)};
            struct Notification
            {
                PathEntry pathEntry;
                std::chrono::steady_clock::time_point expiry;
            };

            using DirectoryEntries = std::unordered_map<std::string, DirectoryEntry>;
            DirectoryEntries m_directoryEntries;

            std::mutex m_pendingActionsLock;
            std::vector<Action> m_pendingActions;

            std::vector<Notification> m_pendingNotifications;

            std::unordered_map<uint64_t, DirectoryEntries::iterator> m_watchIndex;
            uint64_t m_watchId = 0;

            void handleAction(AddWatch& action)
            {
                try
                {
                    auto directory = file_utils::directory(action.path);
                    auto directoryIt = m_directoryEntries.find(directory);

                    if (directoryIt == std::end(m_directoryEntries))
                    {
                        auto handle = createDirectoryHandle(directory);
                        if (!handle)
                            throw std::runtime_error("Failed to create HANDLE");

                        directoryIt = m_directoryEntries.insert(std::make_pair(directory, DirectoryEntry{ directory, *handle })).first;
                    }

                    auto& directoryEntry = directoryIt->second;

                    auto watchId = m_watchId++;
                    PathEntry pathEntry{
                        watchId,
                        action.path,
                        action.onEvent
                    };

                    directoryEntry.pathEntries.push_back(std::move(pathEntry));
                    m_watchIndex.insert(std::make_pair(watchId, directoryIt));
                    action.setValue(WatchId(watchId));
                }
                catch (const std::exception& e)
                {
                    action.setException(e);
                }
            }

            void handleAction(RemoveWatch& action)
            {
                try
                {
                    auto watchId = static_cast<uint64_t>(action.watchId);
                    auto watchIndexIt = m_watchIndex.find(watchId);
                    if (watchIndexIt == std::end(m_watchIndex))
                    {
                        action.setValue(false);
						return;
                    }

                    auto directoryIt = watchIndexIt->second;
                    auto& directoryEntry = directoryIt->second;
                    auto& pathEntries = directoryEntry.pathEntries;

                    auto pathEntryIt = std::find_if(std::begin(pathEntries), std::end(pathEntries), [&](const PathEntry& entry)
                    {
                        return entry.watchId == watchId;
                    });

                    if (pathEntryIt == std::end(pathEntries))
                    {
                        action.setValue(false);
                        return;
                    }

                    pathEntries.erase(pathEntryIt);
                    m_watchIndex.erase(watchIndexIt);

                    action.setValue(true);
                }
                catch (const std::exception& e)
                {
                    action.setException(e);
                }
            }

            static std::optional<HANDLE> createDirectoryHandle(std::string_view path)
            {
                auto handle = CreateFile(
                    path.data(),
                    FILE_LIST_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                    HANDLE(0)
                );

                if (handle == INVALID_HANDLE_VALUE)
                    return std::nullopt;

                return handle;
            }

            static std::string convertWideString(const std::wstring& wstr)
            {
                auto size = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    &wstr[0],
                    static_cast<int>(wstr.size()),
                    nullptr,
                    0,
                    nullptr,
                    nullptr
                );

                if (size <= 0)
                    return "";

                std::string out;
                out.resize(static_cast<size_t>(size));
                WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    &wstr[0],
                    static_cast<int>(wstr.size()),
                    &out[0],
                    size,
                    nullptr,
                    nullptr
                );

                return out;
            }

            void pollDirectoryChanges()
            {
                static constexpr size_t BufferSize { 128 * sizeof(FILE_NOTIFY_INFORMATION) };

                wchar_t filename[MAX_PATH];
                std::vector<BYTE> buffer(BufferSize);

                for (const auto& [_, directoryEntry]: m_directoryEntries)
                {
                    auto handle = directoryEntry.handle;
					DWORD bytesReturned = 0;
                    ReadDirectoryChangesW(
                        handle,
                        buffer.data(),
                        static_cast<DWORD>(BufferSize),
                        FALSE,
                        FILE_NOTIFY_CHANGE_LAST_WRITE,
                        &bytesReturned,
                        nullptr,
                        nullptr
                    );

                	if (bytesReturned > 0)
                    {
                        auto* fileInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(&buffer[0]);
						do
                        {
                            handleDirectoryChange(directoryEntry, fileInfo);
                            fileInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(&buffer[fileInfo->NextEntryOffset]);
                        } while (fileInfo->NextEntryOffset != 0);
                    }
                }
            }

            void handleDirectoryChange(const DirectoryEntry& directoryEntry, const FILE_NOTIFY_INFORMATION* fileInfo)
            {
                std::wstring wideFileName { fileInfo->FileName, fileInfo->FileNameLength / sizeof(wchar_t) };
                auto fileName = convertWideString(wideFileName);

                std::string path(directoryEntry.name);
                path.push_back('\\');
                path.append(fileName);

                const auto& pathEntries = directoryEntry.pathEntries;
                auto pathIt = std::find_if(std::begin(pathEntries), std::end(pathEntries), [&](const PathEntry& pathEntry) {
                    return pathEntry.path == path;
                });

                if (pathIt != std::end(pathEntries))
                    addPendingNotification(*pathIt);
            }

            bool addPendingNotification(const PathEntry& pathEntry)
            {
                auto watchId = pathEntry.watchId;
                auto pendingNotificationIt = std::find_if(std::begin(m_pendingNotifications), std::end(m_pendingNotifications), [&](const Notification& notification) {
                    return notification.pathEntry.watchId == watchId;
                });

                if (pendingNotificationIt != std::end(m_pendingNotifications))
                    return false;

                Notification notification{
                    pathEntry,
                    std::chrono::steady_clock::now() + NotificationDelay
                };

                m_pendingNotifications.push_back(std::move(notification));
                return true;
            }

        	size_t pollPendingNotifications()
            {
                size_t count = 0;

                auto now = std::chrono::steady_clock::now();
                std::vector<std::vector<Notification>::const_iterator> toErase;

                for (auto it = std::begin(m_pendingNotifications); it != std::end(m_pendingNotifications); ++it)
            	{
                    if (now >= it->expiry)
                    {
                        it->pathEntry.onEvent(it->pathEntry.path);
                        ++count;
                        toErase.push_back(it);
                    }
            	}

				for (const auto& it: toErase)
				{
                    m_pendingNotifications.erase(it);
				}

                return count;
            }

        };

        std::shared_ptr<IAsyncQueuePoller> m_poller;
        std::shared_ptr<WatchQueue> m_queue;

    };
}
