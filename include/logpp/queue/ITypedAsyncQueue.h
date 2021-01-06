#pragma once

#include "logpp/queue/IAsyncQueue.h"

namespace logpp
{
    template<typename Entry>
    class ITypedAsyncQueue : public IAsyncQueue
    {
    public:
        using Handler = std::function<void (const Entry&)>;

        virtual void setHandler(const Handler& handler) = 0;

        virtual void push(const Entry& entry) = 0;
        virtual void push(Entry&& entry) = 0;
    };
}