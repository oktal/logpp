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

        void format(std::string_view name, LogLevel level, EventLogBuffer buffer) override
        {
            fmt::memory_buffer formatBuf;

            auto time = buffer.time();
            auto cTime = Clock::to_time_t(time);
            auto utcTime = std::gmtime(&cTime);

            auto getFractionTime = [](TimePoint tp) -> std::pair<std::chrono::milliseconds, std::chrono::microseconds>
            {
                auto epochTp = tp.time_since_epoch();

                epochTp -= std::chrono::duration_cast<std::chrono::seconds>(epochTp);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epochTp);

                epochTp -= std::chrono::duration_cast<std::chrono::milliseconds>(epochTp);
                auto us = std::chrono::duration_cast<std::chrono::microseconds>(epochTp);

                return { ms, us };
            };

            auto [ms, us] = getFractionTime(time);

            Visitor visitor(formatBuf);

            visitor.writeFmt(
                "ts={:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}{:03}",
                utcTime->tm_year + 1900,
                utcTime->tm_mon + 1,
                utcTime->tm_mday,
                utcTime->tm_hour,
                utcTime->tm_min,
                utcTime->tm_sec,
                ms.count(),
                us.count()
            );

            visitor.write("level", levelString(level));
            visitor.write("logger", name);
            visitor.write("msg", buffer.text());

            buffer.visit(visitor);

            m_os.write(formatBuf.data(), formatBuf.size());
            m_os.put('\n');
        }

    private:
        class Visitor : public LogVisitor
        {
        public:
            Visitor(fmt::memory_buffer& buffer)
                : m_formatBuf(buffer)
            {}

            void visit(std::string_view key, std::string_view value) override
            {
                write(key, value);
            }

            void visit(std::string_view key, uint8_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, uint16_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, uint32_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, uint64_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, int8_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, int16_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, int32_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, int64_t value) override
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, float value)
            {
                writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, double value)
            {
                writeFmt("{}={}", key, value);
            }

            void write(std::string_view key, std::string_view value)
            {
                if (value.find(' ') != std::string::npos)
                    writeFmt("{}=\"{}\"", key, value);
                else
                    writeFmt("{}={}", key, value);
            }

            template<typename... Args>
            void writeFmt(const char* formatStr, Args&& ...args)
            {
                if (m_count > 0)
                    m_formatBuf.push_back(' ');

                fmt::format_to(m_formatBuf, formatStr, std::forward<Args>(args)...);
                ++m_count;
            }
        private:
            fmt::memory_buffer& m_formatBuf;
            size_t m_count = 0;

        };

        std::ostream& m_os;
    };
}