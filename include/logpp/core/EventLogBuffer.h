#pragma once

#include "logpp/core/Clock.h"
#include "logpp/core/LogBuffer.h"
#include "logpp/core/LogVisitor.h"
#include "logpp/core/StringLiteral.h"

#include <cstddef>
#include <iostream>

namespace logpp
{
    namespace details
    {
        template< typename T >
        using OffsetT = decltype(static_cast< LogBufferBase* >(nullptr)->write(std::declval< T >()));

        template<typename KeyStr>
        auto writeKey(LogBufferBase& buffer, const KeyStr& key)
        {
            return buffer.write(key);
        }

        inline auto writeKey(LogBufferBase&, StringLiteral key)
        {
            return StringLiteralOffset { key.value };
        }

        template<typename Arg>
        auto write(LogBufferBase& buffer, const Arg& arg)
        {
            auto keyOffset = writeKey(buffer, arg.key);
            auto valueOffset = buffer.write(arg.value);

            return structuredOffset(keyOffset, valueOffset);
        }

        template< typename... Args >
        auto writeAll(LogBufferBase& buffer, Args&& ...args)
        {
            return std::make_tuple(write(buffer, args)...);
        }

        template<typename> struct StructuredEvent;

        #pragma pack(push, 1)
        template<typename... Args>
        struct StructuredEvent<std::tuple<Args...>>
        {
            using Offsets = std::tuple<Args...>;

            StructuredEvent(Offsets offsets)
                : offsets {offsets}
            {}

            void visit(LogBufferView view, LogVisitor& visitor) const
            {
                visitImpl(view, visitor, std::index_sequence_for<Args...>());
            }

        private:
            Offsets offsets;

            template<size_t... Indexes>
            void visitImpl(LogBufferView view, LogVisitor& visitor, std::index_sequence<Indexes...>) const
            {
                auto doVisit = [&](const auto& structuredOffset)
                {
                    auto key = structuredOffset.key.get(view);
                    auto value = structuredOffset.value.get(view);
                    visitor.visit(key, value);
                };

                (doVisit(std::get<Indexes>(offsets)), ...);
            }
        };
        #pragma pack(pop)

        template<typename Offsets>
        StructuredEvent<Offsets> asStructured(Offsets offsets)
        {
            return { offsets };
        }
    }

    template<typename KeyOffset, typename OffsetT>
    struct StructuredOffset
    {
        KeyOffset key;
        OffsetT value;
    };

    template<typename KeyOffset, typename OffsetT>
    StructuredOffset<KeyOffset, OffsetT> structuredOffset(KeyOffset key, OffsetT value)
    {
        return { key, value };
    }

    template<size_t... Indexes, typename Tuple, typename Visitor>
    void visitTuple(std::index_sequence<Indexes...>, Tuple&& tuple, Visitor&& visitor)
    {
        (visitor(std::get<Indexes>(tuple)),...);
    }

    template<typename Tuple>
    constexpr size_t tupleSize(const Tuple&)
    {
        return std::tuple_size_v<Tuple>;
    }

    template<typename Visitor, typename Tuple>
    void visitTuple(Tuple&& tuple, Visitor&& visitor)
    {
        visitTuple(std::make_index_sequence<tupleSize(tuple)>{}, std::forward<Tuple>(tuple), std::forward<Visitor>(visitor));
    }

    class EventLogBuffer : public LogBuffer<255>
    {
    public:
        using VisitFunc = void (*)(const LogBufferBase& buffer, uint16_t offsetsIndex, LogVisitor& visitor);

        static constexpr size_t HeaderOffset = 0;

        struct Header
        {
            TimePoint timePoint;
            StringOffset textOffset;

            uint16_t dataOffsetsIndex;
            VisitFunc visitFunc;
        };

        EventLogBuffer()
        {
            advance(HeaderOffset + sizeof(Header));
        }

        void writeTime(TimePoint timePoint)
        {
            decodeHeader()->timePoint = timePoint;
        }

        template<typename Str>
        void writeText(const Str& str)
        {
            auto offset = this->write(str);
            decodeHeader()->textOffset = offset;
        }

        template<typename... Args>
        void writeData(Args&&... args)
        {
            auto offsets = details::writeAll(*this, std::forward<Args>(args)...);
            auto event = details::asStructured(offsets);

            auto eventOffset = this->encode(event);
            encodeVisitor(event, eventOffset);
        }

        void visit(LogVisitor& visitor) const
        {
            const auto* header = decodeHeader();
            std::invoke(header->visitFunc, *this, header->dataOffsetsIndex, visitor);
        }

        TimePoint time() const
        {
            return decodeHeader()->timePoint;
        }

        std::string_view text() const
        {
            LogBufferView view { *this };
            return decodeHeader()->textOffset.get(view);
        }

    private:
        template<typename Event>
        void encodeVisitor(const Event&, size_t offsetsIndex)
        {
            auto* header         = overlayAt<Header>(HeaderOffset);
            header->dataOffsetsIndex = static_cast<uint16_t>(offsetsIndex);
            header->visitFunc      = [](const LogBufferBase& buffer, uint16_t offsetsIndex, LogVisitor& visitor)
            {
                LogBufferView view{buffer};
                const Event* event = view.overlayAs<Event>(offsetsIndex);
                event->visit(view, visitor);
            };
        }

        Header* decodeHeader()
        {
            return overlayAt<Header>(HeaderOffset);
        }

        const Header* decodeHeader() const
        {
            return overlayAt<Header>(HeaderOffset);
        }
    };
}