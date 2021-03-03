#pragma once

#include "logpp/format/flag/Formatter.h"

namespace logpp
{
    class FieldsFormatter : public FlagFormatter
    {
    public:
        explicit FieldsFormatter(std::string prefix)
            : m_prefix(prefix)
        { }

        void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
        {
            Writer writer(out);
            Visitor visitor(writer, m_prefix);

            buffer.visitFields(visitor);
        }

    private:
        class Writer
        {
        public:
            explicit Writer(fmt::memory_buffer& buffer)
                : m_buf(buffer)
            { }

            void writeRaw(std::string_view str)
            {
                m_buf.append(str.data(), str.data() + str.size());
            }

            void write(std::string_view key, std::string_view value)
            {
                if (value.find(' ') != std::string::npos)
                    writeFmt("{}=\"{}\"", key, value);
                else
                    writeFmt("{}={}", key, value);
            }

            template <typename... Args>
            void writeFmt(const char* formatStr, Args&&... args)
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
            fmt::memory_buffer& m_buf;
            size_t m_count = 0;
        };

        class Visitor : public LogFieldVisitor
        {
        public:
            Visitor(Writer& writer, std::string_view prefix)
                : m_writer(writer)
                , m_prefix(prefix)
            { }

            void visitStart(size_t count) override
            {
                if (count > 0)
                    m_writer.writeRaw(m_prefix);
            }

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

            void visit(std::string_view key, bool value) override
            {
                m_writer.writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, float value) override
            {

                m_writer.writeFmt("{}={}", key, value);
            }

            void visit(std::string_view key, double value) override
            {
                m_writer.writeFmt("{}={}", key, value);
            }

            void visitEnd() override { }

        private:
            Writer& m_writer;
            std::string_view m_prefix;
        };

        std::string m_prefix;
    };
}
