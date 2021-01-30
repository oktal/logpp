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

    bool RollingFileSink::setOption(std::string key, std::string value)
    {
        if (string_utils::iequals(key, "strategy"))
            return setRollingStrategy(createRollingStrategy(value));
        else if (string_utils::iequals(key, "archive"))        
            return setArchiveStrategy(createArchiveStrategy(value));
        return false;
    }

    void RollingFileSink::sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        auto time = buffer.time();
        auto* file = m_file.get();

        if (m_rollingStrategy->apply(time, file))
            m_file.reset(m_archiveStrategy->apply(time, file));

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

    std::pair<std::string_view, std::string_view> RollingFileSink::parseStrategy(std::string_view strategy)
    {
        auto sep = strategy.find('|');

        if (sep == std::string_view::npos)
            return std::make_pair(strategy, strategy);

        return std::make_pair(strategy.substr(sep), strategy.substr(sep + 1));
    }

    std::shared_ptr<ArchiveStrategy> RollingFileSink::createArchiveStrategy(std::string_view strategy)
    {
        auto [name, option] = parseStrategy(strategy);
        if (string_utils::iequals(name, "incremental"))
        {
            return std::make_shared<IncrementalArchiveStrategy>();
        }
        else if (string_utils::iequals(name, "timestamp"))
        {
            if (option.empty())
                option = "utc";

            if (string_utils::iequals(option, "utc"))
                return std::make_shared<TimestampArchiveStrategy<SystemClock>>();
            else if (string_utils::iequals(option, "local"))
                return std::make_shared<TimestampArchiveStrategy<LocalClock>>();
        }

        return nullptr;
    }

    std::shared_ptr<RollingStrategy> RollingFileSink::createRollingStrategy(std::string_view strategy)
    {
        auto [name, option] = parseStrategy(strategy);
        if (string_utils::iequals(name, "size"))
        {
            auto size = string_utils::parseSize(option);
            if (!size)
                return nullptr;

            return std::make_shared<SizeRollingStrategy>(*size);
        }
        return nullptr;
    }
}