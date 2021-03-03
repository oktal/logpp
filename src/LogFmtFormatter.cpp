#include "logpp/format/LogFmtFormatter.h"

#include "logpp/core/Clock.h"
#include "logpp/core/LogFieldVisitor.h"

#include "logpp/utils/date.h"
#include "logpp/utils/thread.h"

#include <chrono>
#include <fmt/format.h>

namespace logpp
{
    class Writer
    {
    public:
        explicit Writer(fmt::memory_buffer& buffer)
            : m_buf(buffer)
        { }

        void write(std::string_view key, std::string_view value)
        {
            if (value.find(' ') != std::string::npos)
                writeFmt(key, "\"{}\"", value);
            else
                writeFmt(key, "{}", value);
        }

        template <typename Val>
        void write(std::string_view key, Val&& value)
        {
            writeFmt(key, "{}", std::forward<Val>(value));
        }

        template <typename... Args>
        void writeFmt(std::string_view key, const char* valueFormatStr, Args&&... args)
        {
            if (m_count > 0)
                m_buf.push_back(' ');

            m_buf.append(key.data(), key.data() + key.size());
            m_buf.push_back('=');
            fmt::format_to(m_buf, valueFormatStr, std::forward<Args>(args)...);
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
        fmt::memory_buffer& m_buf;
        size_t m_count = 0;
    };

    class Visitor : public LogFieldVisitor
    {
    public:
        Visitor(Writer& writer)
            : m_writer(writer)
        { }

        void visitStart(size_t) override { }

        void visit(std::string_view key, std::string_view value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, uint8_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, uint16_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, uint32_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, uint64_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, int8_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, int16_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, int32_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, int64_t value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, bool value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, float value) override
        {
            m_writer.write(key, value);
        }

        void visit(std::string_view key, double value) override
        {
            m_writer.write(key, value);
        }

        void visitEnd() override { }

    private:
        Writer& m_writer;
    };

    void LogFmtFormatter::doFormat(std::string_view name, LogLevel level, const EventLogBuffer& buffer, fmt::memory_buffer& dest) const
    {
        auto tp   = buffer.time();
        auto date = date_utils::date(tp);
        auto time = date_utils::time(tp);

        auto getFractionTime = [](TimePoint tp) -> std::pair<std::chrono::milliseconds, std::chrono::microseconds> {
            auto epochTp = tp.time_since_epoch();

            epochTp -= std::chrono::duration_cast<std::chrono::seconds>(epochTp);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epochTp);

            epochTp -= std::chrono::duration_cast<std::chrono::milliseconds>(epochTp);
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(epochTp);

            return { ms, us };
        };

        auto [ms, us] = getFractionTime(tp);

        Writer writer(dest);
        writer.writeFmt(
            "ts",
            "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}{:03}",
            static_cast<int>(date.year()),
            static_cast<unsigned>(date.month()),
            static_cast<unsigned>(date.day()),
            time.hours().count(),
            time.minutes().count(),
            time.seconds().count(),
            ms.count(),
            us.count());

        writer.write("level", levelString(level));
        writer.write("logger", name);
        writer.write("thread", thread_utils::toInteger(buffer.threadId()));

        fmt::memory_buffer formatBuf;
        buffer.formatText(formatBuf);
        writer.write("msg", std::string_view(formatBuf.data(), formatBuf.size()));

        Visitor visitor(writer);
        buffer.visitFields(visitor);
    }
}
