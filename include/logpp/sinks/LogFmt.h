#pragma once

#include "logpp/sinks/Sink.h"

#include <ostream>
#include <fmt/format.h>

namespace logpp::sink
{
    class LogFmt : public Sink
    {
    public:
        explicit LogFmt(std::ostream &os)
            : m_os(os)
        {}

        void format(LogLevel level, EventLogBuffer buffer, StringOffset text) override
        {
            LogBufferView view { buffer };

            fmt::memory_buffer formatBuf;
            Writer writer { formatBuf };

            writer.write("level", levelString(level));
            writer.write("msg", view, text);
            buffer.format(writer);

            m_os.write(formatBuf.data(), formatBuf.size());
        }

    private:
        class Writer : public LogWriter
        {
        public:
            Writer(fmt::memory_buffer& buffer)
                : m_formatBuf(buffer)
            {}

            void write(std::string_view key, LogBufferView view, StringOffset text) override
            {
                write(key, text.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<uint8_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<uint16_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<uint32_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<uint64_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<int8_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<int16_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<int32_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<int64_t> offset) override
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, LogBufferView view, Offset<double> offset)
            {
                writeFmt("{}={}", key, offset.get(view));
            }

            void write(std::string_view key, std::string_view value)
            {
                if (value.find(' ') != std::string::npos)
                    writeFmt("{}=\"{}\"", key, value);
                else
                    writeFmt("{}={}", key, value);
            }

        private:
            fmt::memory_buffer& m_formatBuf;
            size_t m_count = 0;

            template<typename... Args>
            void writeFmt(const char* formatStr, Args&& ...args)
            {
                if (m_count > 0)
                    m_formatBuf.push_back(' ');

                fmt::format_to(m_formatBuf, formatStr, std::forward<Args>(args)...);
                ++m_count;
            }
        };
        std::ostream& m_os;
    };
}