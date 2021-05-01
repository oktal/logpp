#pragma once

#include "logpp/core/Clock.h"
#include "logpp/core/EventLogBuffer.h"

#include "logpp/core/Logger.h"

#include "logpp/utils/thread.h"

namespace logpp::fuzz
{
    template <typename... Fields>
    struct FuzzStream
    {
        static constexpr auto NFields = sizeof...(Fields);

        FuzzStream(const uint8_t* data, size_t size)
            : data(data)
            , size(size)
        { }

        void fill(EventLogBuffer& buffer)
        {
            auto hasMore = readAs<TimePoint::rep>(
                buffer, [](EventLogBuffer& buffer, auto timePoint) {
                    buffer.writeTime(TimePoint { Clock::duration { timePoint } });
                });

            if (!hasMore)
                return;

            hasMore = readAs<thread_utils::id>(
                buffer, [](EventLogBuffer& buffer, auto threadId) {
                    buffer.writeThreadId(threadId);
                });

            if (!hasMore)
                return;

            const auto textSize = remaining() - sizeOfFields();
            hasMore             = read(
                buffer, textSize,
                [](EventLogBuffer& buffer, const uint8_t* p, size_t size) {
                    buffer.writeText(
                        std::string_view(reinterpret_cast<const char*>(p), size));
                });

            if (!hasMore)
                return;

            fillFields(buffer);
        }

    private:
        template <typename Tuple, size_t Index>
        auto getField()
        {
            using Type          = std::tuple_element_t<Index, Tuple>;
            constexpr auto Size = sizeof(Type);
            static auto stub    = Type();

            auto remainingBytes = remaining();
            if (remainingBytes >= Size)
            {
                auto value = *reinterpret_cast<const Type*>(data);
                m_cursor += Size;
                return logpp::field("Key", value);
            }

            return logpp::field("Key", stub);
        }

        template <typename Tuple, size_t... Indexes>
        void fillFieldsTupleImpl(EventLogBuffer& buffer,
                                 std::index_sequence<Indexes...>)
        {
            buffer.writeFields(getField<Tuple, Indexes>()...);
        }

        template <typename Tuple>
        void fillFieldsTuple(EventLogBuffer& buffer)
        {
            fillFieldsTupleImpl<Tuple>(
                buffer, std::make_index_sequence<std::tuple_size_v<Tuple>> {});
        }

        void fillFields(EventLogBuffer& buffer)
        {
            if constexpr (NFields > 0)
            {
                using Tuple = std::tuple<Fields...>;
                fillFieldsTuple<Tuple>(buffer);
            }
        }

        template <typename Func>
        bool read(EventLogBuffer& buffer, size_t count, Func func)
        {
            const auto remainingBytes = size - m_cursor;
            if (remainingBytes >= count)
            {
                func(buffer, data + m_cursor, count);
                m_cursor += count;
                return true;
            }

            return false;
        }

        template <typename T, typename Func>
        bool readAs(EventLogBuffer& buffer, Func func)
        {
            static constexpr auto Size = sizeof(T);
            return read(
                buffer, Size, [&](EventLogBuffer& buf, const uint8_t* p, size_t) {
                    func(buf, *reinterpret_cast<const T*>(p));
                });
        }

        size_t remaining() const { return size - m_cursor; }

        static constexpr size_t sizeOfFields()
        {
            if constexpr (NFields == 0)
                return 0;
            else
                return (sizeof(Fields) + ...);
        }

        const uint8_t* data;
        size_t size;
        size_t m_cursor { 0 };
    };
}
