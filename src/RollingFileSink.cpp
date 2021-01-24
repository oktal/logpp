#include "logpp/sinks/RollingFileSink.h"

#include "logpp/format/PatternFormatter.h"

namespace logpp::sink
{
    RollingFileSink::RollingFileSink()
        : FileSink()
    {}

    RollingFileSink::RollingFileSink(std::string_view baseFilePath, std::shared_ptr<RollingStrategy> rollingStrategy)
        : RollingFileSink(baseFilePath, std::move(rollingStrategy), std::make_shared<PatternFormatter>("%+"))
    {}

    RollingFileSink::RollingFileSink(std::string_view baseFilePath, std::shared_ptr<RollingStrategy> rollingStrategy, std::shared_ptr<Formatter> formatter)
        : FileSink(baseFilePath, std::move(formatter))
        , m_baseFilePath(baseFilePath)
        , m_rollingStrategy(std::move(rollingStrategy))
    { }

    bool RollingFileSink::setOption(std::string key, std::string value)
    {
        return false;
    }

    void RollingFileSink::sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        fmt::memory_buffer formatBuf;
        format(name, level, buffer, formatBuf);
    }
}