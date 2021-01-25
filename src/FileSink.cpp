#include "logpp/sinks/file/FileSink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/utils/env.h"

namespace logpp::sink
{
    FileSink::FileSink()
        : FormatSink(std::make_shared<PatternFormatter>("%+"))
    { }

    FileSink::FileSink(std::string_view filePath)
        : FileSink(filePath, std::make_shared<PatternFormatter>("%+"))
    {}

    FileSink::FileSink(std::string_view filePath, std::shared_ptr<Formatter> formatter)
        : FormatSink(std::move(formatter))
    {
        open(filePath);
    }

    bool FileSink::setOption(std::string key, std::string value)
    {
        auto optionsBase = FormatSink::setOption(key, value);
        if (optionsBase)
            return true;

        if (key == "file")
        {
            open(env_utils::expandEnvironmentVariables(value));
            return true;
        }

        return false;
    }

    bool FileSink::open(std::string_view filePath)
    {
        return m_file->open(filePath, std::ios_base::out | std::ios_base::app);
    }

    bool FileSink::isOpen() const
    {
        return m_file->isOpen();
    }

    bool FileSink::close()
    {
        return m_file->close();
    }

    void FileSink::sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        fmt::memory_buffer formatBuf;
        format(name, level, buffer, formatBuf);
        m_file->write(formatBuf.data(), formatBuf.size());
    }
}