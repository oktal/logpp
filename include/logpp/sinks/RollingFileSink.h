#pragma once

#include "logpp/sinks/FormatSink.h"
#include "logpp/sinks/FileSink.h"

#include "logpp/utils/file.h"

#include <chrono>
#include <ctime>

namespace logpp::sink
{
    enum class RollingInterval
    {
        Minute,
        Hour,
        Day,
        Month,
        Year,
        Infinite
    };

    class RollingFile : public File
    {
    public:
    };

    class RollingStrategy
    {
    public:
        virtual ~RollingStrategy() = default;

        virtual bool apply(TimePoint tp, const File* file) = 0;
    };

    class SizeRollingStrategy : public RollingStrategy
    {
    public:
        explicit SizeRollingStrategy(size_t bytesThreshold)
            : bytesThreshold(bytesThreshold)
        {}

        bool apply(TimePoint, const File* file)
        {
            return file->size() >= bytesThreshold;
        }

        size_t bytesThreshold;
    };

    class DateRollingStrategy : public RollingStrategy
    {
    public:
        explicit DateRollingStrategy(RollingInterval interval)
            : interval(interval)
        {}

        bool apply(TimePoint tp, const File*)
        {
            if (interval == RollingInterval::Infinite)
                return false;

            if (!nextRollTime)
            {
                nextRollTime = computeNextRollTime(tp);
            }
            else if (tp >= *nextRollTime)
            {
                nextRollTime = computeNextRollTime(*nextRollTime);
                return true;
            }

            return false;
        }

    private: 
        RollingInterval interval;
        std::optional<TimePoint> nextRollTime;

        TimePoint computeNextRollTime(TimePoint time) const
        {
            switch (interval)
            {
                case RollingInterval::Minute:
                    return time + std::chrono::minutes(1);
                case RollingInterval::Hour:
                    return time + std::chrono::hours(1);
            }

            return time;
        }
    };

    class CompositeRollingStrategy : public RollingStrategy
    {
    public:
        explicit CompositeRollingStrategy(std::vector<std::shared_ptr<RollingStrategy>> strategies)
            : m_strategies(std::move(strategies))
        {}

        template<typename It>
        explicit CompositeRollingStrategy(It begin, It end)
        {
            std::copy(begin, end, std::back_inserter(m_strategies));
        }

        explicit CompositeRollingStrategy(std::initializer_list<std::shared_ptr<RollingStrategy>> strategies)
            : CompositeRollingStrategy(std::begin(strategies), std::end(strategies))
        {}

        bool apply(TimePoint tp, const File* file)
        {
            auto res = false;
            for (auto& strategy: m_strategies)
            {
                res |= strategy->apply(tp, file);
            }
            return res;
        }

    private:
        std::vector<std::shared_ptr<RollingStrategy>> m_strategies;
    };

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

    class RollingFileSink : public FileSink
    {
    public:
        static constexpr std::string_view Name = "RollingFileSink";

        RollingFileSink();
        RollingFileSink(std::string_view baseFilePath, std::shared_ptr<RollingStrategy> rollingStrategy);
        RollingFileSink(std::string_view baseFilePath, std::shared_ptr<RollingStrategy> rollingStrategy, std::shared_ptr<Formatter> formatter);

        bool setOption(std::string key, std::string value) override;

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override;

    private:
        std::string m_baseFilePath;
        std::shared_ptr<File> m_currentFile;

        std::shared_ptr<RollingStrategy> m_rollingStrategy;
    };
}