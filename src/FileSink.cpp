#include "logpp/sinks/FileSink.h"

#include "logpp/format/PatternFormatter.h"

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

    bool FileSink::open(std::string_view filePath)
    {
        auto openMode = std::ios_base::out | std::ios_base::app;
        auto fs = std::make_unique<std::ofstream>(filePath.data(), openMode);
        if (fs->bad())
            return false;

        std::swap(m_fs, fs);
        return true;
    }

    bool FileSink::isOpen() const
    {
        return m_fs && m_fs->is_open();
    }

    bool FileSink::close()
    {
        if (!m_fs)
            return false;

        m_fs->close();
        m_fs.reset();

        return true;
    }

    void FileSink::sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer)
    {
        m_formatBuf.clear();
        format(name, level, buffer, m_formatBuf);
        m_fs->write(m_formatBuf.data(), m_formatBuf.size());
        m_fs->put('\n');
    }

}