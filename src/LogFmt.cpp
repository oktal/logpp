#include "logpp/core/Clock.h"
#include "logpp/sinks/LogFmt.h"

#include <chrono>
#include <fmt/format.h>

namespace logpp::sink
{
    class Writer
    {
    public:
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
                m_buf.push_back(' ');

            fmt::format_to(m_buf, formatStr, std::forward<Args>(args)...);
            ++m_count;
        }

        const char* data() const
        {
            return m_buf.data();
        }

        size_t size() const
        {
            return m_buf.size();
        }

    private:
        fmt::memory_buffer m_buf;
        size_t m_count = 0;
    };

    class Visitor : public LogVisitor
    {
    public:
        Visitor(Writer& writer)
            : m_writer(writer)
        {}

        void visit(std::string_view key, std::string_view value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, uint8_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, uint16_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, uint32_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, uint64_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, int8_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, int16_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, int32_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, int64_t value) override
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, float value)
        {
            m_writer.writeFmt("{}={}", key, value);
        }

        void visit(std::string_view key, double value)
        {
            m_writer.writeFmt("{}={}", key, value);
        }

    private:
        Writer& m_writer;
    };

    LogFmt::LogFmt(std::ostream& os)
        : m_os(os)
    {}

    void LogFmt::format(std::string_view name, LogLevel level, EventLogBuffer buffer)
    {
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

        Writer writer;
        writer.writeFmt(
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

        writer.write("level", levelString(level));
        writer.write("logger", name);

        fmt::memory_buffer formatBuf;
        buffer.formatText(formatBuf);
        writer.write("msg", std::string_view(formatBuf.data(), formatBuf.size()));

        Visitor visitor(writer);
        buffer.visit(visitor);

        m_os.write(writer.data(), writer.size());
        m_os.put('\n');
    }
}
