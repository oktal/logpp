#include "logpp/logpp.h"

#include "logpp/format/flag/Formatter.h"
#include "logpp/format/PatternFormatter.h"

#include "logpp/sinks/ColoredConsole.h"

using namespace logpp;

// A flag formatter that formats log message fields as JSON
class JsonFormatter : public FlagFormatter
{
public:
    explicit JsonFormatter(std::string prefix)
        : m_prefix(std::move(prefix))
    {}

    void format(std::string_view, LogLevel, const EventLogBuffer& buffer, fmt::memory_buffer& out) const override
    {
        Writer writer(out);
        Visitor visitor(writer, m_prefix);
        buffer.visitFields(visitor);
    }

private:
    struct Writer
    {
        explicit Writer(fmt::memory_buffer& out)
            : out(out)
        {}

        void writeRaw(std::string_view str)
        {
            out.append(str);
        }

        template<typename T>
        void write(std::string_view key, T&& value)
        {
            writeKey(key);
            writeValue(std::forward<T>(value));
        }

        void writeKey(std::string_view key)
        {
            using namespace std::string_view_literals;
            if (count > 0)
                out.append(", "sv);

            fmt::format_to(out, "\"{}\": ", key);
            ++count;
        }

        template<typename T>
        void writeValue(T&& value)
        {
            fmt::format_to(out, "{}", value);
        }

        void writeValue(std::string_view str)
        {
            fmt::format_to(out, "\"{}\"", str);
        }

        void writeValue(bool value)
        {
            if (value)
                writeRaw("true");
            else
                writeRaw("false");
        }

        fmt::memory_buffer& out;
        size_t count = 0;
    };

    struct Visitor : public LogFieldVisitor
    {
        Visitor(Writer& writer, std::string_view prefix)
            : writer(writer)
            , prefix(prefix)
        {}

        void visitStart(size_t fieldsCount) override
        {
            if (fieldsCount > 0)
            {
                writer.writeRaw(prefix);
                writer.writeRaw("{ ");
            }
            count = fieldsCount;
        }

        void visit(std::string_view key, std::string_view value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, uint8_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, uint16_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, uint32_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, uint64_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, int8_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, int16_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, int32_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, int64_t value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, bool value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, float value) override
        {
            writer.write(key, value);
        }

        void visit(std::string_view key, double value) override
        {
            writer.write(key, value);
        }

        void visitEnd() override
        {
            if (count > 0)
                writer.writeRaw(" }");
        }

        Writer& writer;
        std::string_view prefix;
        size_t count;
    };

    std::string m_prefix;
};

int main()
{
    // Register our custom flag formatter
    PatternFormatter::registerFlag<JsonFormatter>('j');

    // Create a pattern formatter with our custom flag
    auto formatter = std::make_shared<PatternFormatter>("%Y-%m-%d %H:%M:%S [%l] (%n) %v%j[ - ]");

    // Create a console output sink with our formatter
    auto consoleOut = std::make_shared<sink::ColoredOutputConsole>(formatter);

    // Create our logger
    auto logger = std::make_shared<Logger>("main", LogLevel::Info, consoleOut);

    // Log a message with some fields
    logger->info("Something happened.",
        logpp::field("Id", 1234),
        logpp::field("IsValid", true),
        logpp::field("Name", "Something")
    );
}
