#include "logpp/queue/IAsyncQueue.h"

#include <cstddef>
#include <type_traits>

namespace logpp
{

    class IGenericAsyncQueue : public IAsyncQueue
    {
    public:
        struct HandlerBase
        {
            virtual ~HandlerBase() = default;
        };

        template <typename Entry>
        struct Handler : HandlerBase
        {
            virtual ~Handler() = default;

            virtual void handleEntry(Entry entry) = 0;
        };

        using Constructor    = void (*)(void*, const void*);
        using HandlerInvoker = void (*)(const void*, size_t, HandlerBase*);

        template <typename Entry>
        void push(Entry&& entry, Handler<std::decay_t<Entry>>* handler)
        {
            using CleanEntry             = std::decay_t<Entry>;
            static constexpr size_t Size = sizeof(CleanEntry);

            static auto invoker = [](const void* entryBuffer, size_t size, HandlerBase* handler) {
                if (size < sizeof(CleanEntry))
                    return;

                auto typedHandler = static_cast<Handler<CleanEntry>*>(handler);
                typedHandler->handleEntry(*reinterpret_cast<const CleanEntry*>(entryBuffer));
            };

            static auto constructor = [](void* mem, const void* entryBytes) {
                ::new (mem) CleanEntry(*reinterpret_cast<const CleanEntry*>(entryBytes));
            };

            pushImpl(std::addressof(entry), Size, constructor, handler, invoker);
        }

    private:
        virtual void pushImpl(const void* entryBytes, size_t size, Constructor constructor, HandlerBase* handler, HandlerInvoker invoker) = 0;
    };
}