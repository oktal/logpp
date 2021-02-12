#pragma once

#include "logpp/sinks/FormatSink.h"
#include "logpp/sinks/file/File.h"

#include <memory>
#include <string_view>

namespace logpp::sink
{
    class FileSink : public FormatSink
    {
    public:
        static constexpr std::string_view Name = "FileSink";

        FileSink();
        FileSink(std::string_view filePath);
        FileSink(std::string_view filePath, std::shared_ptr<Formatter> formatter);

        bool activateOptions(const Options& options) override;

        bool open(std::string_view filePath);
        bool isOpen() const;

        bool close();

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override;

    protected:
        std::unique_ptr<File> m_file;

        virtual void onAfterOpened(const std::unique_ptr<File>&) {}
        virtual void onBeforeClosing(const std::unique_ptr<File>&) {}
    };
}