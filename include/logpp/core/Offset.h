#pragma once

#include "logpp/core/LogBufferView.h"

#include <functional>

namespace logpp
{
    template< typename T >
    struct Offset
    {
        Offset()
            : offset{0}
        {}

        explicit Offset(uint16_t offset)
            : offset{offset}
        {}

        T get(LogBufferView buffer) const
        {
            return buffer.readAs< T >(offset);
        }

    private:
        uint16_t offset;
    };


    #define INTEGER_OFFSET_TYPES \
        INTEGER_OFFSET(uint8_t)  \
        INTEGER_OFFSET(uint16_t) \
        INTEGER_OFFSET(uint32_t) \
        INTEGER_OFFSET(uint64_t) \
        INTEGER_OFFSET(int8_t)   \
        INTEGER_OFFSET(int16_t)  \
        INTEGER_OFFSET(int32_t)  \
        INTEGER_OFFSET(int64_t)  \

    namespace tag
    {
        struct String
        {};

        struct Function
        {};
    }

    template<>
    struct Offset< tag::String >
    {
        Offset()
            : offset{0}
        {}

        explicit Offset(uint16_t offset)
            : offset{offset}
        {}

        std::string_view get(LogBufferView buffer) const
        {
            auto size        = buffer.readAs< uint16_t >(offset);
            const char* data = buffer.read(offset + sizeof(uint16_t));
            return std::string_view(data, size);
        }

        const char* data(LogBufferView buffer) const
        {
            return buffer.read(offset + sizeof(uint16_t));
        }

        size_t size(LogBufferView buffer) const
        {
            return buffer.readAs< uint16_t >(offset);
        }


    private:
        uint16_t offset;
    };

    template<>
    struct Offset< tag::Function >
    {
        Offset()
            : offset{0}
        {}

        explicit Offset(uint16_t offset)
            : offset{offset}
        {}

        template< typename FuncType, typename... Args >
        auto invoke(LogBufferView buffer, Args&& ...args) const
        {
            auto* func = buffer.overlayAs< FuncType >(offset);
            std::invoke(*func, std::forward< Args >(args)...);
        }

    private:
        uint16_t offset;
    };

    using StringOffset = Offset< tag::String >;
    using FunctionOffset = Offset< tag::Function >;
}