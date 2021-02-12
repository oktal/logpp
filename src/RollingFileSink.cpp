#include "logpp/sinks/file/RollingFileSink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/utils/string.h"

namespace logpp::sink
{

    RollingFileSink::RollingFileSink()
        : FileSink()
    {}

    RollingFileSink::RollingFileSink(
        std::string_view baseFilePath,
        std::shared_ptr<Formatter> formatter,
        std::shared_ptr<RollingStrategy> rollingStrategy,
        std::shared_ptr<ArchiveStrategy> archiveStrategy
    )
        : FileSink(baseFilePath, std::move(formatter))
        , m_baseFilePath(baseFilePath)
        , m_rollingStrategy(std::move(rollingStrategy))
        , m_archiveStrategy(std::move(archiveStrategy))
    { }

    bool RollingFileSink::activateOptions(const Options& options)
    {
        if (!FileSink::activateOptions(options))
            return false;

        auto strategyOpts = options.tryGet("strategy");
        auto archiveOpts = options.tryGet("archive");

        if (!strategyOpts || !archiveOpts)
            return false;

        return setRollingStrategy(createRollingStrategy(*strategyOpts)) &&
               setArchiveStrategy(createArchiveStrategy(*archiveOpts));
    }

    void RollingFileSink::sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        auto time = buffer.time();
        auto* file = m_file.get();

        if (m_rollingStrategy->apply(time, file))
        {
            onBeforeClosing(m_file);

            m_file.reset(m_archiveStrategy->apply(time, file));

            onAfterOpened(m_file);
        }

        FileSink::sink(name, level, buffer);
    }

    bool RollingFileSink::setArchiveStrategy(std::shared_ptr<ArchiveStrategy> strategy)
    {
        if (!strategy)
            return false;

        m_archiveStrategy = std::move(strategy);
        return true;
    }

    bool RollingFileSink::setRollingStrategy(std::shared_ptr<RollingStrategy> strategy)
    {
        if (!strategy)
            return false;

        m_rollingStrategy = std::move(strategy);
        return true;
    }

    std::shared_ptr<ArchiveStrategy> RollingFileSink::createArchiveStrategy(const Options::Value& options)
    {
        if (auto opts = options.asString())
        {
            if (string_utils::iequals(*opts, "incremental"))
                return std::make_shared<IncrementalArchiveStrategy>();
            else if (string_utils::iequals(*opts, "timestamp"))
                return std::make_shared<TimestampArchiveStrategy<SystemClock>>();
        }
        else if (auto opts = options.asDict())
        {
            auto typeIt = opts->find("type");
            if (typeIt == std::end(*opts))
                return nullptr;

            auto type = typeIt->second;

            if (string_utils::iequals(type, "incremental"))
            {
                return std::make_shared<IncrementalArchiveStrategy>();
            }
            else if (string_utils::iequals(type, "timestamp"))
            {
                auto clockIt = opts->find("clock");
                if (clockIt == std::end(*opts))
                    return std::make_shared<TimestampArchiveStrategy<SystemClock>>();

                auto clock = clockIt->second;
                if (string_utils::iequals(clock, "utc"))
                    return std::make_shared<TimestampArchiveStrategy<SystemClock>>();
                else if (string_utils::iequals(clock, "local"))
                    return std::make_shared<TimestampArchiveStrategy<LocalClock>>();
            }
        }

        return nullptr;
    }

    std::shared_ptr<RollingStrategy> RollingFileSink::createRollingStrategy(const Options::Value& options)
    {
        if (auto opts = options.asDict())
        {
            auto typeIt = opts->find("type");
            if (typeIt == std::end(*opts))
                return nullptr;

            auto type = typeIt->second;
            if (string_utils::iequals(type, "size"))
            {
                auto sizeIt = opts->find("size");
                if (sizeIt == std::end(*opts))
                    return nullptr;

                auto size = string_utils::parseSize(sizeIt->second);
                if (!size)
                    return nullptr;

                return std::make_shared<SizeRollingStrategy>(*size);
            }
            else if (string_utils::iequals(type, "date"))
            {
                auto intervalIt = opts->find("interval");
                if (intervalIt == std::end(*opts))
                    return nullptr;

                auto kindIt = opts->find("kind");
                if (kindIt == std::end(*opts))
                    return nullptr;

                auto interval = tryParseRollingInterval(intervalIt->second);
                auto kind = tryParseRollingKind(kindIt->second);

                if (!interval || !kind)
                    return nullptr;

                return std::make_shared<DateRollingStrategy>(*interval, *kind);
            }
        }

        return nullptr;
    }
}