#pragma once

#include "Abc.Shared/DateTime.h"
#include "Abc.Shared/DateTimeHelper.h"
#include "Abc.Shared/JsonTextWriter.h"
#include "Abc.Shared/IMessageLogWriter.h"

#include "Abc.Shared/IAsyncQueue.h"
#include "Abc.Shared/IAsyncQueuePoller.h"

#include <fmt/format.h>
#include <tuple>

/*
 * It is time for a better, low-latency, logging framework.
 *
 * This logging component is intended to be used in critical code sections, where performance really matters.

 * The whole process is asynchronous, meaning that formatting will be done asynchronously in a background thread.
 * The basic idea of the logger is to maintain a `LogBuffer`, where the desired data to log is serialized.
 * However, unlike other logging frameworks that rely on the same principle, only the bare minimum of data is 
 * serialized into the `LogBuffer` (the dynamic part of the data).
 * The whole idea to minimize strings serialization is to serialize an object that holds relative offsets to the data
 * written to the `LogBuffer` inside the `LogBuffer`, so that this object can be later used to perform the desired formatting.
 * 
 * The `LogBuffer` is structured as such:

      +------------------------+-----------------+----------+
      | Header                 |                 |          |
      |---------+--------------|   Data Block    |  Offsets |
      | LogFunc | OffsetsIndex |                 |    |     |
      +---------+--------------+-----------------+----|-----+
                    |                 ^---------------|  ^
                    |                                    |
                    |-------------------------------------
                    

 * The header contains the index of the `Offsets` block inside the buffer and is followed by the block of data. The offsets to the data
 * encoded in the Data Block then end the buffer.
 *
 * When writing something to the Data Block, the `LogBuffer` will return an `Offset<T>`, which is a tiny typed wrapper containing the relative
 * offset of the data being encoded inside the Data Block.
 * This `Offset<T>` must then be placed inside a type so that it can later on be serialized inside the `LogBuffer`.
 *
 * Let's suppose we want to log something everytime an user triggers a new timer.
 *
 * (1) First, let's declare a struct that will hold the offsets to the data of this particular timer event;
 *
 *   struct TimerEvent
 *   {
 *       Offset<uint32_t> timerId;
 *       Offset<uint64_t> period;
 *       StringOffset description;
 *   };

 * (2) Then, we serialize the data the the LogBuffer and fill the TimerEvent
 *
 *  EventLogBuffer buffer;
 *  TimerEvent timerEvent {
 *      buffer.write(myTimer.id()),
 *      buffer.write(myTimer.period()),
 *      buffer.write("My beautiful timer"),
 *  };
 *
 * (3) Our timerEvent now holds offsets to the data corresponding to the timer, serialized inside the EventLogBuffer, we finally write the timerEvent, which is our "Offsets"
 *     inside the LogBuffer:
 *
 *  buffer.writeEvent(timerEvent);
 *
 * (4) To be able to correctly format our TimerEvent, the last thing to do is to implement a static `format` function with the givent signature:
 *
 *    static void format(LogBufferView view, FormatCallback& onFormat);
 *
 * (5) Here is our final TimerEvent

 *   struct TimerEvent
 *   {
 *       Offset<uint32_t> timerId;
 *       Offset<uint64_t> period;
 *       StringOffset description;
 *
 *       static void format(LogBufferView view, FormatCallback& onFormat)
 *       {
 *           std::ostringstream oss;
 *           oss << "TimerId: " << timerId.get(view) << " Period: " << timerPeriod.get(view) << " Description: " << description.get(view);
 *
 *           auto str = oss.str();
 *           onFormat(str.data(), str.size());
 *       }
 *   };
 *
 *  
 *  To make things easier, the library provides a `logEvent` function. To use it,
 *
 *  (1) Declare and start an EventLogger
 *
 *     EventLogger logger;
 *     logger.setPattern("%Y-%m-%d %H:%M:%S %t %E")
 *     logger.start();
 *
 * (2) Call logEvent<Event>
 *
 *     loEvent<TimerEvent>(logger, "Timer started.", [&](LogBufferBase& buffer, auto& event) {
 *         event.timerId = buffer.write(myTimer.id());
 *         event.period = buffer.write(myTimer.period());
 *         event.description = buffer.write("My beautiful timer");
 *      });
 *
 *  Alternatively, you can use the `logF` variant to log data through a lambda. That way, you don't need to declare and fill an obbject, the logging framework
 *  will take care of that for you and will call your lambda back.
 *
 *    logF(logger, "Timer started.", [](LogBufferView buffer, FormatCallback& onFormat, Offset<uint32_t> timerId, Offset<uint32_t> period, StringOffset description) {
 *        std::ostringstream oss;
 *        oss << "TimerId: " << timerId.get(view) << " Period: " << timerPeriod.get(view) << " Description: " << description.get(view);
 *
 *        auto str = oss.str();
 *        onFormat(str.data(), str.size());
 *     }, myTimer.id(), myTimer.period(), "My beautiful timer");
 *
*/


namespace Abc::Shared
{
    template< typename Func >
    struct FunctionInvoker
    {
        explicit FunctionInvoker(LogBufferView view, FunctionOffset offset)
            : view{view},
              offset{offset}
        {}

        template< typename... Args >
        void invoke(Args&& ...args)
        {
            offset.invoke< Func >(view, std::forward< Args >(args)...);
        }

    private:
        LogBufferView view;
        FunctionOffset offset;
    };

    class FormatCallback
    {
    public:
        virtual ~FormatCallback() = default;

        void operator()(const char* data, size_t size)
        {
            call(data, size);
        }

    private:
        virtual void call(const char* data, size_t size) = 0;
    };

    class EventLogBuffer : public LogBuffer< 255 >
    {
    public:
        using LogFunc = void (*)(const LogBufferBase& buffer, uint16_t offsetsIndex, FormatCallback& onFormat);
        static constexpr size_t HeaderOffset = 0;

        struct Header
        {
            LogFunc logFunc;
            uint16_t offsetsIndex;
        };

        EventLogBuffer();

        template< typename Event >
        void writeEvent(const Event& event)
        {
            auto offsetsIndex = encode(event);
            encodeHeader< Event >(offsetsIndex);
        }

        void format(FormatCallback& onFormat) const;

    private:
        template< typename Event >
        void encodeHeader(size_t offsetsIndex)
        {
            auto* header         = overlayAt< Header >(HeaderOffset);
            header->offsetsIndex = static_cast< uint16_t >(offsetsIndex);
            header->logFunc      = [](const LogBufferBase& buffer, uint16_t offsetsIndex, FormatCallback& onFormat)
            {
                LogBufferView view{buffer};
                const Event* event = view.overlayAs< Event >(offsetsIndex);
                event->format(view, onFormat);
            };
        }

        const Header* decodeHeader() const
        {
            return overlayAt< Header >(HeaderOffset);
        }
    };

    class IEventLogger
    {
    public:
        virtual ~IEventLogger() {}

        virtual void setPattern(const std::string& pattern) = 0;
        virtual void registerWriter(const std::shared_ptr< IMessageLogWriter >& writer) = 0;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual void log(StringOffset text, const EventLogBuffer& buffer) = 0;
    };

    class EventLogger final : public IEventLogger
    {
    public:
        EventLogger(std::shared_ptr< IAsyncQueuePoller > queuePoller);
        ~EventLogger();

        void setPattern(const std::string& pattern) override;
        void registerWriter(const std::shared_ptr< IMessageLogWriter >& writer) override;

        void start() override;
        void stop() override;

        void log(StringOffset textOffset, const EventLogBuffer& buffer) override;

    private:
        class FmtCallback : public FormatCallback
        {
        public:
            explicit FmtCallback(fmt::memory_buffer& buffer)
                : buffer{buffer}
            {}

        private:
            void call(const char* data, size_t size) override
            {
                format_to(buffer, "{}", fmt::string_view(data, size));
            }

            fmt::memory_buffer& buffer;
        };

        struct Entry
        {
            static Entry create(StringOffset textOffset, const EventLogBuffer& buffer)
            {
                return Entry{DateTime::utcNow(), textOffset, buffer};
            }

            void format(fmt::memory_buffer& buffer) const
            {
                FmtCallback callback{buffer};
                logBuffer.format(callback);
            }

            DateTime date;
            StringOffset textOffset;
            EventLogBuffer logBuffer;
        };

        std::shared_ptr< ITypedAsyncQueue< Entry > > m_queue;
        std::shared_ptr< IAsyncQueuePoller > m_queuePoller;

        using PatternAction = std::function< void (fmt::memory_buffer& buffer, const Entry& entry) >;
        std::string m_pattern;

        std::vector< std::shared_ptr< IMessageLogWriter > > m_writers;
        std::vector< PatternAction > m_patternActions;
        fmt::memory_buffer m_buf;

        void parsePattern();

        std::pair< PatternAction, std::string::const_iterator > parseFormat(std::string::const_iterator it, std::string::const_iterator end);
        std::pair< PatternAction, std::string::const_iterator > parseEventFormat(std::string::const_iterator it, std::string::const_iterator end);

        void handleEntry(fmt::memory_buffer& buffer, const Entry& entry);
    };

    namespace details
    {
        template< typename T >
        using OffsetT = decltype(static_cast< LogBufferBase* >(nullptr)->write(std::declval< T >()));
        template< typename... Args >
        using OffsetTuple = std::tuple< OffsetT< std::decay_t< Args > >... >;

        template< typename... Args >
        auto writeAll(LogBufferBase& buffer, Args&& ...args)
        {
            return std::make_tuple(buffer.write(args)...);
        }

        struct DefaultAdapter
        {
            struct AutoEnabler
            {
                template< typename... Args >
                using Type = void (*)(LogBufferView, FormatCallback&, Args ...);
            };

            template< typename Func, typename... Args >
            static void format(LogBufferView view, FormatCallback& callback, FunctionInvoker< Func > invoker, Args&& ...args)
            {
                invoker.invoke(view, callback, std::forward< Args >(args)...);
            }
        };

        #pragma pack(push, 1)
        template< typename Func, typename Adapter, typename... Args >
        struct EventWrapper
        {
            using Offsets = OffsetTuple< Args... >;

            EventWrapper(Offsets offsets, FunctionOffset funcOffset)
                : offsets{offsets},
                  funcOffset{funcOffset}
            {}

            void format(LogBufferView view, FormatCallback& callback) const
            {
                formatImpl(view, callback, std::index_sequence_for< Args... >());
            }

        private:
            Offsets offsets;
            FunctionOffset funcOffset;

            template< size_t... Indexes >
            void formatImpl(LogBufferView view, FormatCallback& callback, std::index_sequence< Indexes... >) const
            {
                FunctionInvoker< Func > invoker{view, funcOffset};
                Adapter::format(view, callback, invoker, std::get< Indexes >(offsets)...);
            }
        };
        #pragma pack(pop)

        /*
            Utils to deal with lambdas.

            To properly handle lambda in `logF` api, we need to be able to deduce the "real" type and make the lambda decay
            to its function-pointer equivalent.

            We need that because we will then serialize the _address_ of the lambda as a function-pointer inside the LogBuffer,
            so that we can reinterpret_cast it later-on to the right type.

            However, lambdas do not automatically decay to a function-pointer like an array would decay to a const pointer to its first element.
            We then need a way to _force_ the lambda to decay. To do that, we use the operator+ trick. For a given lambda

            [](int a), operator+ applied to the lambda +[](int a) will yield a function-pointer of type void (*)(int).

            This works well, except for generic lambdas. For a given lambda [](auto a, auto b), the compiler won't be able to deduce the function-pointer
            for the lambda as the compiler-generated function for the lambda is templated.

            For a custom adapter to support generic lambdas, the user must provide an `AutoEnabler` with an inner `Type` typedef'ed to the expected type
            of the underlying lambda, e.g.

            struct StringAdapter
            {
                struct AutoEnabler
                {
                    template<typename... Args>
                    using Type = void (*)(LogBufferView, std::string&, Args...);
                };

                template<typename Func, typename... Args>
                static void format(LogBufferView buffer, FormatCallback& callback, FunctionInvoker<Func> invoker, Args&& ...args)
                {
                    std::string str;
                    invoker.invoke(buffer, str, std::forward<Args>(args)...);
                    callback(str.data(), str.size());
                }
            };

            Note how the line typedef'ing the lambda perfectly matches the `invoker.invoke` part:

            template<typename... Args>
            using Type = void (*)(LogBufferView,    std::string&,    Args...);
                                     ^                ^                ^
                             --------   ---------------      ----------
                            |          |                    |
            invoker.invoke(buffer,    str,    std::forward<Args>(args)...);
             


            If the user does not provide an `AutoEnabler` inner type, we will fallback and still be able to decay the lambda to its function-pointer equivalent.

            However, in that case, using a generic lambda won't be supported and the code won't compiler.
        */

        template< typename T >
        struct FunctionPtr;

        template< typename R, typename... Args >
        struct FunctionPtr< R (*)(Args ...) >
        {
            using Type = R (*)(Args ...);

            explicit FunctionPtr(Type func)
                : func(func)
            {}

            FunctionOffset write(LogBufferBase& buffer)
            {
                return buffer.write(func);
            }

        private:
            Type func;
        };

        template< typename Adapter, typename Func, typename... Args >
        auto functionPtrForImpl(Func func, typename Adapter::AutoEnabler*)
        {
            using AutoEnabler = typename Adapter::AutoEnabler;
            using FuncType = typename AutoEnabler::template Type< OffsetT< Args >... >;

            return details::FunctionPtr< FuncType >{func};
        }

        template< typename Adapter, typename Func, typename... Args >
        auto functionPtrForImpl(Func func, ...)
        {
            return details::FunctionPtr< decltype(+func) >{func};
        }

        template< typename Adapter, typename Func, typename... Args >
        auto functionPtrFor(Func func)
        {
            // Use SFINAE to dispatch to the implementation, depending whether or not the user provided an inner `AutoEnabler` type.
            return functionPtrForImpl< Adapter, Func, Args... >(func, nullptr);
        }
    }

    struct JsonAdapter
    {
        struct AutoEnabler
        {
            template< typename... Args >
            using Type = void (*)(LogBufferView, JsonTextWriter&, Args ...);
        };

        template< typename Func, typename... Args >
        static void format(LogBufferView view, FormatCallback& onFormat, FunctionInvoker< Func > invoker, Args&& ...args)
        {
            fmt::memory_buffer buf;
            JsonTextWriter writer{buf};
            {
                JsonTextWriter::ObjectContext object{writer};
                invoker.invoke(view, writer, std::forward< Args >(args)...);
            }

            onFormat(buf.data(), buf.size());
        }
    };

    template< typename Event, typename EventFunc >
    void logEvent(const std::shared_ptr< IEventLogger >& logger, const char* text, EventFunc&& func)
    {
        if (!logger)
            return;

        logEvent< Event >(*logger, text, std::forward< EventFunc >(func));
    }

    template< typename Event, typename EventFunc >
    void logEvent(IEventLogger& logger, const char* text, EventFunc&& eventFunc)
    {
        EventLogBuffer buffer;
        Event event;
        std::invoke(std::forward< EventFunc >(eventFunc), buffer, event);

        auto textOffset = buffer.write(text);
        buffer.writeEvent(event);

        logger.log(textOffset, buffer);
    }

    template< typename Adapter, typename Func, typename... Args >
    void logF(const std::shared_ptr< IEventLogger >& logger, const char* text, Func func, Args&& ...args)
    {
        if (!logger)
            return;

        logF< Adapter >(*logger, text, func, std::forward< Args >(args)...);
    }

    template< typename Func, typename... Args >
    void logF(const std::shared_ptr< IEventLogger >& logger, const char* text, Func func, Args&& ...args)
    {
        logF< details::DefaultAdapter >(logger, text, func, std::forward< Args >(args)...);
    }

    template< typename Adapter, typename Func, typename... Args >
    void logF(IEventLogger& logger, const char* text, Func func, Args&& ...args)
    {
        auto funcPtr = details::functionPtrFor< Adapter, Func, Args... >(func);

        EventLogBuffer buffer;

        auto offsets    = details::writeAll(buffer, std::forward< Args >(args)...);
        auto funcOffset = funcPtr.write(buffer);

        details::EventWrapper< Func, Adapter, Args... > event(offsets, funcOffset);
        auto textOffset = buffer.write(text);

        buffer.writeEvent(event);
        logger.log(textOffset, buffer);
    }

    template< typename Func, typename... Args >
    void logF(IEventLogger& logger, const char* text, Func func, Args&& ...args)
    {
        logF< details::DefaultAdapter >(logger, text, func, std::forward< Args >(args)...);
    }
} // namespace Abc::Shared
