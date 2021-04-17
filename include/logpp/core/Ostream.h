#pragma once

#include "logpp/core/EventLogBuffer.h"
#include "logpp/utils/detect.h"

#include <fmt/format.h>
#include <ostream>

namespace logpp
{
    namespace details
    {
        template <typename Char>
        class MemoryStreamBuf : public std::basic_streambuf<Char>
        {
        public:
            MemoryStreamBuf(fmt::basic_memory_buffer<Char>& buffer)
                : m_buffer(buffer)
            { }

            using int_type    = typename std::basic_streambuf<Char>::int_type;
            using traits_type = typename std::basic_streambuf<Char>::traits_type;

        private:
            fmt::basic_memory_buffer<Char>& m_buffer;

        protected:
            int_type overflow(int_type ch = traits_type::eof()) override
            {
                if (!traits_type::eq_int_type(ch, traits_type::eof()))
                    m_buffer.push_back(static_cast<Char>(ch));
                return ch;
            }

            std::streamsize xsputn(const Char* s, std::streamsize count) override
            {
                m_buffer.append(s, s + count);
                return count;
            }
        };

        template <typename T>
        using HasStream = decltype(std::declval<std::ostream&>() << std::declval<T>());

        template <typename T>
        constexpr bool IsStreamable = is_detected_v<HasStream, T>;
    }

    template <typename Key, typename Value>
    struct FieldWriter<Key, Value, std::enable_if_t<details::IsStreamable<Value>>>
    {
        template <typename Buffer>
        auto write(Buffer& buffer, const Key& key, const Value& value)
        {
            fmt::memory_buffer buf;
            details::MemoryStreamBuf<char> streamBuf(buf);
            std::basic_ostream<char> os(&streamBuf);

            os << value;
            auto keyOffset   = buffer.write(key);
            auto valueOffset = buffer.write(std::string_view(buf.data(), buf.size()));

            return std::make_pair(keyOffset, valueOffset);
        }
    };
}
