#include "logpp/sinks/FormatSink.h"

#include <fstream>
#include <memory>
#include <string_view>

namespace logpp::sink
{
    class FileSink : public FormatSink
    {
    public:
        FileSink();
        explicit FileSink(std::string_view filePath);
        explicit FileSink(std::string_view filePath, std::shared_ptr<Formatter> formatter);

        bool open(std::string_view filePath);
        bool isOpen() const;

        bool close();

        void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer);

    private:
        std::unique_ptr<std::ofstream> m_fs;
        fmt::memory_buffer m_formatBuf;
    };
}