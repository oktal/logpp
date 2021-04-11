#include "gtest/gtest.h"

#include "logpp/core/LogBuffer.h"

using namespace logpp;

TEST(LogBuffer, should_write_basic_types_to_log_buffer)
{
    LogBuffer<255> buffer;

    auto off1 = buffer.write(static_cast<uint8_t>(12));
    auto off2 = buffer.write(static_cast<uint16_t>(345));
    auto off3 = buffer.write(static_cast<uint32_t>(6789));
    auto off4 = buffer.write(static_cast<uint64_t>(123456789));

    auto off5 = buffer.write(static_cast<char>('A'));
    auto off6 = buffer.write(true);

    LogBufferView view { buffer };

    ASSERT_EQ(off1.get(view), 12);
    ASSERT_EQ(off2.get(view), 345);
    ASSERT_EQ(off3.get(view), 6789);
    ASSERT_EQ(off4.get(view), 123456789);
    ASSERT_EQ(off5.get(view), 'A');
    ASSERT_EQ(off6.get(view), true);
}

TEST(LogBuffer, should_write_string_to_log_buffer)
{
    LogBuffer<255> buffer;

    auto off1       = buffer.write("Literal");
    auto off2       = buffer.write(std::string("Standard"));
    const char* raw = "Raw";
    auto off3       = buffer.write(raw);

    LogBufferView view { buffer };

    ASSERT_EQ(off1.get(view), "Literal");
    ASSERT_EQ(off2.get(view), "Standard");
    ASSERT_EQ(off3.get(view), "Raw");
}

TEST(LogBuffer, should_copy_and_keep_offsets)
{
    LogBuffer<255> buffer;

    auto off1 = buffer.write(static_cast<uint64_t>(0xDEADBEEF));
    auto off2 = buffer.write("Literal string");
    auto off3 = buffer.write('B');

    LogBuffer<255> bufferCopy(buffer);
    LogBufferView view { bufferCopy };

    ASSERT_EQ(off1.get(view), 0xDEADBEEF);
    ASSERT_EQ(off2.get(view), "Literal string");
    ASSERT_EQ(off3.get(view), 'B');
}

TEST(LogBuffer, should_move_and_keep_offsets)
{
    LogBuffer<255> buffer;

    auto off1 = buffer.write(static_cast<uint64_t>(0xDEADBEEF));
    auto off2 = buffer.write("Literal string");
    auto off3 = buffer.write('B');

    LogBuffer<255> bufferMove { std::move(buffer) };
    LogBufferView view { bufferMove };

    ASSERT_EQ(off1.get(view), 0xDEADBEEF);
    ASSERT_EQ(off2.get(view), "Literal string");
    ASSERT_EQ(off3.get(view), 'B');
}

TEST(LogBuffer, should_grow_when_small_and_full)
{
    LogBuffer<8> buffer;

    auto off1 = buffer.write(static_cast<uint32_t>(0xDEAD));
    auto off2 = buffer.write(static_cast<uint32_t>(0xBEEF));
    auto off3 = buffer.write(static_cast<uint64_t>(0xDEADBEEF));
    auto off4 = buffer.write("The beef is dead");

    LogBufferView view { buffer };

    ASSERT_EQ(off1.get(view), 0xDEAD);
    ASSERT_EQ(off2.get(view), 0xBEEF);
    ASSERT_EQ(off3.get(view), 0xDEADBEEF);
    ASSERT_EQ(off4.get(view), "The beef is dead");
}

TEST(LogBuffer, should_copy_and_keep_offsets_after_growing)
{
    LogBuffer<8> buffer;

    auto off1 = buffer.write(static_cast<uint32_t>(0xDEAD));
    auto off2 = buffer.write(static_cast<uint32_t>(0xBEEF));
    auto off3 = buffer.write(static_cast<uint64_t>(0xDEADBEEF));
    auto off4 = buffer.write("The beef is dead");

    LogBuffer<8> bufferCopy { buffer };
    LogBufferView view { bufferCopy };

    ASSERT_EQ(off1.get(view), 0xDEAD);
    ASSERT_EQ(off2.get(view), 0xBEEF);
    ASSERT_EQ(off3.get(view), 0xDEADBEEF);
    ASSERT_EQ(off4.get(view), "The beef is dead");
}

TEST(LogBuffer, should_move_and_keep_offsets_after_growing)
{
    LogBuffer<8> buffer;

    auto off1 = buffer.write(static_cast<uint32_t>(0xDEAD));
    auto off2 = buffer.write(static_cast<uint32_t>(0xBEEF));
    auto off3 = buffer.write(static_cast<uint64_t>(0xDEADBEEF));
    auto off4 = buffer.write("The beef is dead");

    LogBuffer<8> bufferMove { std::move(buffer) };
    LogBufferView view { bufferMove };

    ASSERT_EQ(off1.get(view), 0xDEAD);
    ASSERT_EQ(off2.get(view), 0xBEEF);
    ASSERT_EQ(off3.get(view), 0xDEADBEEF);
    ASSERT_EQ(off4.get(view), "The beef is dead");
}