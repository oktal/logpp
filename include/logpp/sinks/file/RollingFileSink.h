#pragma once

#include "logpp/sinks/FormatSink.h"
#include "logpp/sinks/file/FileSink.h"

#include "logpp/sinks/file/ArchiveStrategy.h"
#include "logpp/sinks/file/RollingStrategy.h"

namespace logpp::sink
{

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