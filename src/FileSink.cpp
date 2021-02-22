#include "logpp/sinks/file/FileSink.h"

#include "logpp/format/PatternFormatter.h"
#include "logpp/utils/env.h"

namespace logpp::sink
{
    class FileImpl : public File
    {
    public:
        explicit FileImpl(std::string_view filePath, std::ios_base::openmode openMode)
        {
            open(filePath, openMode);
        }

        ~FileImpl()
        {
            if (isOpen())
                close();
        }

        bool open(std::string_view filePath, std::ios_base::openmode openMode)
        {
            if (m_ofs)
                return false;

            auto ofs = std::make_unique<std::ofstream>(filePath.data(), openMode);
            if (ofs->bad() || !ofs->is_open())
                return false;

            std::swap(m_ofs, ofs);
            m_path = filePath;
            return true;
        }

        bool isOpen() const override
        {
            return m_ofs && m_ofs->is_open();
        }

        bool close() override
        {
            if (!m_ofs)
                return false;

            m_ofs->close();
            m_ofs.reset();

            return true;
        }

        size_t write(const char* data, size_t size) override
        {
            if (!m_ofs)
                return 0ULL;

            m_ofs->write(data, size);
            return size;
        }

        size_t write(const char c) override
        {
            if (!m_ofs)
                return 0ULL;

            m_ofs->put(c);
            return 1ULL;
        }

        size_t size() const override
        {
            if (!m_ofs || !m_ofs->rdbuf())
                return 0;

            return  m_ofs->rdbuf()->pubseekoff(0, std::ios_base::cur, std::ios_base::out);
        }

        void flush() override
        {
            if (!m_ofs)
                return;

            m_ofs->flush();
        }

    private:
        std::unique_ptr<std::ofstream> m_ofs;
    };

    FileSink::FileSink()
        : FormatSink(std::make_shared<PatternFormatter>("%+"))
    { }

    FileSink::FileSink(std::string_view filePath)
        : FileSink(filePath, std::make_shared<PatternFormatter>("%+"))
    { }

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
        m_file.reset(new FileImpl(filePath, std::ios_base::out | std::ios_base::app));
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
