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
        static constexpr std::string_view Name = "RollingFile";

        RollingFileSink();
        RollingFileSink(
            std::string_view baseFilePath,
            std::shared_ptr<Formatter> formatter,
            std::shared_ptr<RollingStrategy> rollingStrategy,
            std::shared_ptr<ArchiveStrategy> archiveStrategy
        );

        bool activateOptions(const Options& options) override;

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override;

    private:
        std::string m_baseFilePath;

        std::shared_ptr<RollingStrategy> m_rollingStrategy;
        std::shared_ptr<ArchiveStrategy> m_archiveStrategy;

    protected:
        bool setArchiveStrategy(std::shared_ptr<ArchiveStrategy> strategy);
        bool setRollingStrategy(std::shared_ptr<RollingStrategy> strategy);

        virtual std::shared_ptr<ArchiveStrategy> createArchiveStrategy(const Options::Value& options);
        virtual std::shared_ptr<RollingStrategy> createRollingStrategy(const Options::Value& options);
    };
}