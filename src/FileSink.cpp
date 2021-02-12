#include "logpp/sinks/file/FileSink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/utils/env.h"

namespace logpp::sink
{
    FileSink::FileSink()
        : FormatSink(std::make_shared<PatternFormatter>("%+"))
    {}

    FileSink::FileSink(std::string_view filePath)
        : FileSink(filePath, std::make_shared<PatternFormatter>("%+"))
    {}

    FileSink::FileSink(std::string_view filePath, std::shared_ptr<Formatter> formatter)
        : FormatSink(std::move(formatter))
    {
        open(filePath);
    }

    bool FileSink::activateOptions(const Options& options)
    {
        if (!FormatSink::activateOptions(options))
            return false;

        auto fileOption = options.tryGet("file");
        if (!fileOption)
            return false;

        auto file = fileOption->asString();
        if (!file)
            return false;

        open(env_utils::expandEnvironmentVariables(*file));
        return true;
    }

    bool FileSink::open(std::string_view filePath)
    {
        m_file.reset(new File(filePath, std::ios_base::out | std::ios_base::app));
        onAfterOpened(m_file);
        return isOpen();
    }

    bool FileSink::isOpen() const
    {
        if (!m_file)
            return false;
        return m_file->isOpen();
    }

    bool FileSink::close()
    {
        if (!m_file)
            return false;

        onBeforeClosing(m_file);
        return m_file->close();
    }

    void FileSink::sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        if (!m_file)
            return;

        fmt::memory_buffer formatBuf;
        format(name, level, buffer, formatBuf);
        m_file->write(formatBuf.data(), formatBuf.size());
        m_file->write('\n');
        m_file->flush();
    }
}