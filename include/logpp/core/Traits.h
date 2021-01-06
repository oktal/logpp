#pragma once

#include <tuple>

namespace logpp
{

    namespace impl
    {
        template<typename> struct LogFunctionTraits;

        template<typename Ret, typename Class, typename... Args>
        struct LogFunctionTraits<Ret (Class::*)(Args...) const>
        {
            template<size_t N> using ArgAt = std::tuple_element_t<N, std::tuple<Args...>>;

            static_assert(
                sizeof...(Args) == 2,
                "Log function must have signature void (LogBufferBase&, TEvent&)"
            );

            //static_assert(
            //    std::is_same_v<ArgAt<0>, LogBufferBase&>,
            //    "Log function must have signature void (LogBufferBase&, TEvent&)"
            //);

            using Event = std::decay_t<ArgAt<1>>;
        };
    }

    template<typename Func>
    using LogFunctionTraits = impl::LogFunctionTraits<decltype(&Func::operator())>;
}