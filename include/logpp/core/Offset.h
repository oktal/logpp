#pragma once

#include "logpp/core/LogBufferView.h"

#include <cstring>
#include <functional>

#include <iostream>

namespace logpp
{
#if !defined(LOGPP_OFFSET_TYPE)
  #define LOGPP_OFFSET_TYPE uint32_t
#endif

    using OffsetType = LOGPP_OFFSET_TYPE;

    template <typename T>
    struct Offset
    {
        Offset()
            : offset { 0 }
        { }

        explicit Offset(OffsetType offset)
            : offset { offset }
        { }

        T get(LogBufferView buffer) const
        {
            return buffer.readAs<T>(offset);
        }

    private:
        OffsetType offset;
    };

    namespace tag
    {
        struct String
        { };

        struct StringLiteral
        { };

        template <typename T>
        struct Ptr
        { };

        struct Function
        { };
    }

    template <>
    struct Offset<tag::String>
    {
        Offset()
            : offset { 0 }
        { }

        explicit Offset(OffsetType offset)
            : offset { offset }
        { }

        std::string_view get(LogBufferView buffer) const
        {
            auto size        = buffer.readAs<OffsetType>(offset);
            const char* data = buffer.read(offset + sizeof(OffsetType));
            return std::string_view(data, size);
        }

        const char* data(LogBufferView buffer) const
        {
            return buffer.read(offset + sizeof(OffsetType));
        }

        size_t size(LogBufferView buffer) const
        {
            return buffer.readAs<OffsetType>(offset);
        }

    private:
        OffsetType offset;
    };

    template <>
    struct Offset<tag::StringLiteral>
    {
        Offset()
            : offset { 0 }
        { }

        explicit Offset(OffsetType offset)
            : offset { offset }
        { }

        std::string_view get(LogBufferView buffer) const
        {
            auto ptr = buffer.readAs<uintptr_t>(offset);
            return std::string_view(reinterpret_cast<const char*>(ptr));
        }

    private:
        OffsetType offset;
    };

    template <typename T>
    struct Offset<tag::Ptr<T>>
    {
        Offset()
            : offset { 0 }
        { }

        explicit Offset(OffsetType offset)
            : offset { offset }
        { }

        T* get(LogBufferView buffer)
        {
            return buffer.overlayAs<T>(offset);
        }

    private:
        OffsetType offset;
    };

    template <>
    struct Offset<tag::Function>
    {
        Offset()
            : offset { 0 }
        { }

        explicit Offset(OffsetType offset)
            : offset { offset }
        { }

        template <typename FuncType, typename... Args>
        auto invoke(LogBufferView buffer, Args&&... args) const
        {
            auto* func = buffer.overlayAs<FuncType>(offset);
            std::invoke(*func, std::forward<Args>(args)...);
        }

    private:
        OffsetType offset;
    };

    template <typename T>
    using PtrOffset           = Offset<tag::Ptr<T>>;
    using StringOffset        = Offset<tag::String>;
    using StringLiteralOffset = Offset<tag::StringLiteral>;
    using FunctionOffset      = Offset<tag::Function>;
}
