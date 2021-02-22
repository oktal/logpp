#pragma once

#include "logpp/sinks/FormatSink.h"
#include "logpp/sinks/file/FileSink.h"

#include "logpp/sinks/file/RollingOfstream.h"

namespace logpp::sink
{

    class RollingFileSink : public FileSink
    {
    public:
        static constexpr std::string_view Name = "RollingFile";

        RollingFileSink();
        RollingFileSink(
            std::string_view baseFilePath,
            std::shared_ptr<Formatter> formatter);

        bool activateOptions(const Options& options) override;

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override;
    private:
        std::string m_baseFilePath;
    };
}
