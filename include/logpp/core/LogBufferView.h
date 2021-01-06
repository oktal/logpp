#pragma once

#include <cstddef>

namespace logpp
{
    class LogBufferBase;

    class LogBufferView
    {
    public:
        explicit LogBufferView(const LogBufferBase& buffer)
            : buffer{buffer}
        {}

        const char* read(size_t index) const;

        template< typename T >
        T readAs(size_t index) const
        {
            return *reinterpret_cast< const T* >(read(index));
        }

        template< typename T >
        const T* overlayAs(size_t index) const
        {
            return reinterpret_cast< const T* >(read(index));
        }

    private:
        const LogBufferBase& buffer;
    };
}