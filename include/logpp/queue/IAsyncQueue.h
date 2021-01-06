#pragma once

#include <cstddef>

namespace logpp
{
    class IAsyncQueue
    {
    public:
        virtual ~IAsyncQueue() = default;

        virtual size_t pollOne() = 0;
        virtual size_t poll() = 0;
    };
}