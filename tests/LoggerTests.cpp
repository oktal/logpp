#include "logpp/core/LogFieldVisitor.h"
#include "logpp/core/Logger.h"
#include "logpp/core/Ostream.h"

#include "logpp/sinks/Sink.h"
#include "logpp/utils/detect.h"

#include "gtest/gtest.h"

using namespace logpp;

struct Field
{
    virtual ~Field() = default;

    explicit Field(std::string key)
        : key(std::move(key))
    { }

    std::string key;
};

template <typename T>
struct TypedField : Field
{
    TypedField(std::string key, T value)
        : Field(std::move(key))
        , value(std::move(value))
    { }

    T value;
};

template <>
struct TypedField<std::string_view> : Field
{
    TypedField(std::string key, std::string_view value)
        : Field(std::move(key))
        , value(std::string(value))
    { }

    std::string value;
};

struct LogEntry
{
    LogEntry(std::string name, LogLevel level, std::string text,
             std::vector<std::shared_ptr<Field>> fields)
        : name(std::move(name))
        , level(level)
        , text(std::move(text))
        , fields(std::move(fields))
    { }

    std::string name;
    LogLevel level;

    std::string text;
    std::vector<std::shared_ptr<Field>> fields;

    template <typename T>
    std::vector<std::shared_ptr<TypedField<T>>> findFields() const
    {
        std::vector<std::shared_ptr<TypedField<T>>> res;

        for (const auto& field : fields)
        {
            if (auto typedField = std::dynamic_pointer_cast<TypedField<T>>(field);
                typedField != nullptr)
                res.push_back(typedField);
        }
        return res;
    }

    template <typename T>
    std::shared_ptr<TypedField<T>> findField(std::string_view key) const
    {
        for (const auto& field : fields)
        {
            if (field->key != key)
                continue;

            if (auto typedField = std::dynamic_pointer_cast<TypedField<T>>(field);
                typedField != nullptr)
                return typedField;
        }

        return nullptr;
    }
};

class MemorySink : public sink::Sink
{
public:
    void activateOptions(const sink::Options&) override
    {
    }

    void sink(std::string_view name, LogLevel level, const EventLogBuffer& buffer) override
    {
        fmt::memory_buffer textBuf;
        buffer.formatText(textBuf);

        Visitor visitor;
        buffer.visitFields(visitor);

        m_entries.emplace_back(std::string(name), level, std::string(textBuf.data(), textBuf.size()), visitor.fields());
    }

    const LogEntry* findEntry(std::string_view text, LogLevel level) const
    {
        auto entryIt = std::find_if(
            std::begin(m_entries), std::end(m_entries), [&](const auto& entry) {
                return entry.text == text && entry.level == level;
            });

        if (entryIt == std::end(m_entries))
            return nullptr;

        return &*entryIt;
    }

private:
    class Visitor : public LogFieldVisitor
    {
    public:
        void visitStart(size_t count) override
        {
            m_fields.reserve(count);
        }

#define DEFINE_VISIT(ValueType)                                \
    void visit(std::string_view key, ValueType value) override \
    {                                                          \
        onField(key, value);                                   \
    }

        DEFINE_VISIT(std::string_view)
        DEFINE_VISIT(uint8_t)
        DEFINE_VISIT(uint16_t)
        DEFINE_VISIT(uint32_t)
        DEFINE_VISIT(uint64_t)
        DEFINE_VISIT(int8_t)
        DEFINE_VISIT(int16_t)
        DEFINE_VISIT(int32_t)
        DEFINE_VISIT(int64_t)
        DEFINE_VISIT(bool)
        DEFINE_VISIT(float)
        DEFINE_VISIT(double)

#undef DEFINE_VISIT

        void visitEnd() override
        { }

        template <typename T>
        void onField(std::string_view key, const T& value)
        {
            m_fields.push_back(std::make_shared<TypedField<T>>(std::string(key), value));
        }

        std::vector<std::shared_ptr<Field>> fields() const
        {
            return m_fields;
        }

    private:
        std::vector<std::shared_ptr<Field>> m_fields;
    };

    std::vector<LogEntry> m_entries;
};

struct LoggerTest : public ::testing::Test
{
    void SetUp() override
    {
        sink   = std::make_shared<MemorySink>();
        logger = createLogger("LoggerTest", LogLevel::Debug);
    }

    std::shared_ptr<Logger> createLogger(std::string name, LogLevel level) const
    {
        return std::make_shared<Logger>(std::move(name), level, sink);
    }

    const LogEntry* findEntry(std::string_view text, LogLevel level) const
    {
        return sink->findEntry(text, level);
    }

    const LogEntry* checkEntry(std::string_view text, LogLevel level) const
    {
        auto* entry = sink->findEntry(text, level);
        EXPECT_NE(entry, nullptr);
        return entry;
    }

    template <typename T>
    std::shared_ptr<TypedField<T>> checkField(const LogEntry* entry, std::string_view key, T value)
    {
        auto field = entry->findField<T>(key);

        EXPECT_NE(field, nullptr);
        EXPECT_EQ(field->key, key);
        EXPECT_EQ(field->value, value);

        return field;
    }

    std::shared_ptr<MemorySink> sink;
    std::shared_ptr<Logger> logger;
};

TEST_F(LoggerTest, should_log_message_depending_on_log_level)
{
    auto logger = createLogger("LoggerTest", LogLevel::Info);

    struct TestData
    {
        const char* message;
        LogLevel level;

        bool shouldBeLogged;
    } testData[] = {
        { "This is a trace message", LogLevel::Trace, false },
        { "This is a debug message", LogLevel::Debug, false },
        { "This is an info message", LogLevel::Info, true },
        { "This is a warning message", LogLevel::Warning, true },
        { "This is an error message", LogLevel::Error, true }
    };

    for (const auto& data : testData)
    {
        logger->log(data.message, data.level);
        auto* entry = findEntry(data.message, data.level);

        if (data.shouldBeLogged && !entry)
            FAIL() << "Message SHOULD have been logged";
        if (!data.shouldBeLogged && entry)
            FAIL() << "Message SHOULD NOT have been logged";
    }
}

TEST_F(LoggerTest, should_log_message_with_fields)
{
    logger->info("Test message",
                 logpp::field("status_code", uint32_t(0xBAD)),
                 logpp::field("status_text", std::string("BAD")));

    auto* entry = checkEntry("Test message", LogLevel::Info);
    checkField(entry, "status_code", uint32_t(0xBAD));
    checkField(entry, "status_text", std::string_view("BAD"));

    ASSERT_EQ(entry->findField<uint16_t>("status_code"), nullptr);
}

struct TestId
{
    explicit TestId(std::string name, uint32_t id)
        : name(std::move(name))
        , id(id)
    { }

    std::string name;
    uint32_t id;
};

std::ostream& operator<<(std::ostream& os, const TestId& id)
{
    os << id.name;
    os << ':';
    os << id.id;
    return os;
}

TEST_F(LoggerTest, should_log_message_with_custom_streamable_fields)
{
    TestId id("LoggerTest.should_log_message_with_custom_streamable_fields", 0xABC);

    logger->info("Test message",
                 logpp::field("test_id", id));

    auto* entry = checkEntry("Test message", LogLevel::Info);

    std::ostringstream oss;
    oss << id;

    checkField(entry, "test_id", std::string_view(oss.str()));
}
