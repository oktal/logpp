#pragma once

#include "logpp/core/Clock.h"
#include "logpp/core/FormatArgs.h"
#include "logpp/core/LogBuffer.h"
#include "logpp/core/LogVisitor.h"
#include "logpp/core/StringLiteral.h"

#include <cstddef>
#include <iostream>

#include <fmt/format.h>

namespace logpp
{
    namespace details
    {
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

        template< typename T >
        using OffsetT = decltype(static_cast< LogBufferBase* >(nullptr)->write(std::declval< T >()));

        template<typename Str>
        auto writeString(LogBufferBase& buffer, const Str& str)
        {
            return buffer.write(str);
        }

        inline auto writeString(LogBufferBase&, StringLiteral str)
        {
            return StringLiteralOffset { str.value };
        }

        template<typename Arg>
        auto write(LogBufferBase& buffer, const Arg& arg)
        {
            auto keyOffset = writeString(buffer, arg.key);
            auto valueOffset = buffer.write(arg.value);

            return structuredOffset(keyOffset, valueOffset);
        }

        template< typename... Args >
        auto writeAll(LogBufferBase& buffer, Args&& ...args)
        {
            return std::make_tuple(write(buffer, args)...);
        }

        template<typename Tuple, size_t... Indexes>
        auto writeArgsImpl(LogBufferBase& buffer, const Tuple& args, std::index_sequence<Indexes...>)
        {
            return std::make_tuple(buffer.write(std::get<Indexes>(args))...);
        }

        template<typename...Args>
        auto writeArgs(LogBufferBase& buffer, const std::tuple<Args...>& args)
        {
            return writeArgsImpl(buffer, args, std::make_index_sequence<sizeof...(Args)>{});
        }

        template<typename> struct TextEvent;

        template<typename... Args>
        struct TextEvent<std::tuple<Args...>>
        {
            using ArgsOffsets = std::tuple<Args...>;

            TextEvent(StringOffset formatStrOffset, ArgsOffsets argsOffsets)
                : formatStrOffset { formatStrOffset }
                , argsOffsets { argsOffsets }
            {}

            void formatTo(fmt::memory_buffer& buffer, LogBufferView view) const
            {
                auto formatStr = formatStrOffset.get(view);
                formatImpl(view, buffer, formatStr, std::make_index_sequence<sizeof...(Args)>{});
            }

        private:
            StringOffset formatStrOffset;
            ArgsOffsets argsOffsets;

            template<size_t... Indexes>
            void formatImpl(LogBufferView view, fmt::memory_buffer& buffer, std::string_view formatStr, std::index_sequence<Indexes...>) const
            {
                fmt::format_to(buffer, formatStr, getArg<Indexes>(view)...);
            }

            template<size_t N>
            auto getArg(LogBufferView view) const
            {
                auto offset = std::get<N>(argsOffsets);
                return offset.get(view);
            }
        };

        template<typename> struct StructuredEvent;

        #pragma pack(push, 1)
        template<typename... Args>
        struct StructuredEvent<std::tuple<Args...>>
        {
            using Offsets = std::tuple<Args...>;

            StructuredEvent(Offsets offsets)
                : offsets { offsets }
            {}

            void visit(LogBufferView view, LogVisitor& visitor) const
            {
                visitTuple(offsets, [&](const auto& structuredOffset) {
                    auto key = structuredOffset.key.get(view);
                    auto value = structuredOffset.value.get(view);
                    visitor.visit(key, value);
                });
            }

            constexpr size_t count() const
            {
                return sizeof...(Args);
            }

        private:
            Offsets offsets;
        };
        #pragma pack(pop)

        template<typename Offsets>
        StructuredEvent<Offsets> asStructured(Offsets offsets)
        {
            return { offsets };
        }

        template<typename ArgsOffsets>
        TextEvent<ArgsOffsets> asText(StringOffset textOffset, ArgsOffsets argsOffsets)
        {
            return { textOffset, argsOffsets };
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

    class EventLogBuffer : public LogBuffer<256>
    {
    public:
        using TextFormatFunc = void (*)(const LogBufferBase& buffer, uint16_t offsetsIndex, fmt::memory_buffer& formatBuf);
        using VisitFunc = void (*)(const LogBufferBase& buffer, uint16_t offsetsIndex, LogVisitor& visitor);

        static constexpr size_t HeaderOffset = 0;

        struct Header
        {
            TimePoint timePoint;

            uint16_t textOffsetsIndex;
            TextFormatFunc formatFunc;

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

            auto event = details::asText(offset, std::make_tuple());
            auto eventOffset = this->encode(event);

            encodeTextFormater(event, eventOffset);
        }

        template<typename... Args>
        void writeText(const FormatArgsHolder<Args...>& holder)
        {
            auto formatStrOffset = this->write(holder.formatStr);
            auto argsOffsets = details::writeArgs(*this, holder.args);

            auto event = details::asText(formatStrOffset, argsOffsets);
            auto eventOffset = this->encode(event);

            encodeTextFormater(event, eventOffset);
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

        void formatText(fmt::memory_buffer& buffer) const
        {
            const auto* header = decodeHeader();
            std::invoke(header->formatFunc, *this, header->textOffsetsIndex, buffer);
        }

    private:
        template<typename Event>
        void encodeTextFormater(const Event&, size_t offsetsIndex)
        {
            auto* header         = overlayAt<Header>(HeaderOffset);
            header->textOffsetsIndex = static_cast<uint16_t>(offsetsIndex);
            header->formatFunc      = [](const LogBufferBase& buffer, uint16_t offsetsIndex, fmt::memory_buffer& formatBuf)
            {
                LogBufferView view{buffer};
                const Event* event = view.overlayAs<Event>(offsetsIndex);
                event->formatTo(formatBuf, view);
            };
        }

        template<typename Event>
        void encodeVisitor(const Event&, size_t offsetsIndex)
        {
            auto* header         = overlayAt<Header>(HeaderOffset);
            header->dataOffsetsIndex = static_cast<uint16_t>(offsetsIndex);
            header->visitFunc      = [](const LogBufferBase& buffer, uint16_t offsetsIndex, LogVisitor& visitor)
            {
                LogBufferView view{buffer};
                const Event* event = view.overlayAs<Event>(offsetsIndex);

                visitor.visitStart(event->count());
                event->visit(view, visitor);
                visitor.visitEnd();
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