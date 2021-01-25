#pragma once

#include "logpp/sinks/file/File.h"
#include "logpp/utils/file.h"

#include <ctime>
#include <fmt/format.h>
#include <string>

namespace logpp::sink
{
    class ArchiveStrategy
    {
    public:
        virtual ~ArchiveStrategy() = default;

        virtual File* apply(File* logFile) = 0;
    };

    class IncrementalArchiveStrategy : public ArchiveStrategy
    {
    public:

        File* apply(File* logFile) override
        {
            auto basePath = logFile->path();
            archive(basePath);
            return new File(basePath, std::ios_base::out);
        }

        static int archive(std::string_view basePath)
        {
            int n = -1;
            std::string path;
            do
            {
                path = fmt::format("{}.{}", basePath, ++n);
            } while (file_utils::exists(path));

            auto index = n;

            while (n >= 0)
            {
                auto oldPath = n > 0 ? fmt::format("{}.{}", basePath, n - 1) : std::string(basePath);
                auto newPath = fmt::format("{}.{}", basePath, n);
                file_utils::move(oldPath, newPath);
                --n;
            }

            return index;
        }
    };

    template<typename Clock>
    class TimestampArchiveStrategy : public ArchiveStrategy
    {
    public:
        static constexpr std::string_view DefaultPattern = "%Y%m%d";

        explicit TimestampArchiveStrategy(std::string_view pattern = DefaultPattern)
            : m_pattern(pattern)
        { }

        std::string pattern() const
        {
            return m_pattern;
        }

        File* apply(File* logFile) override
        {
            auto basePath = logFile->path();
            auto newPath = std::string(basePath);
            newPath.push_back('.');

            auto now = Clock::now();
            auto tp = Clock::to_time_t(now);

            char buf[255];
            auto size = std::strftime(buf, sizeof(buf), m_pattern.c_str(), std::gmtime(&tp));

            newPath.append(buf, size);

            if (file_utils::exists(newPath))
                IncrementalArchiveStrategy::archive(newPath);

            file_utils::move(basePath, newPath);
            return new File(basePath, std::ios_base::out);
        }

    private:
        std::string m_pattern;
    };
}