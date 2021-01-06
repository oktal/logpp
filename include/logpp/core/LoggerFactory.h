#pragma once

#include "logpp/core/Logger.h"

namespace logpp
{

    class LoggerFactory
    {
    public:
        template<typename T>
        static std::shared_ptr<Logger> getLogger(std::shared_ptr<sink::Sink> sink)
        {
            return getLogger(std::string(nameOf<T>()), std::move(sink));
        }

        static std::shared_ptr<Logger> getLogger(std::string name, std::shared_ptr<sink::Sink> sink)
        {
            return std::make_shared<Logger>(std::move(name), LogLevel::Debug, std::move(sink));
        }

    private:
        template <typename T>
        static constexpr auto nameOf() noexcept {
            std::string_view name = "Error: unsupported compiler", prefix, suffix;
            #ifdef LOGPP_COMPILER_CLANG
                name = __PRETTY_FUNCTION__;
                prefix = "auto type_name() [T = ";
                suffix = "]";
            #elif defined(LOGPP_COMPILER_GCC)
                name = __PRETTY_FUNCTION__;
                prefix = "constexpr auto type_name() [with T = ";
                suffix = "]";
            #elif defined(LOGPP_COMPILER_MSVC)
                name = __FUNCSIG__;
                prefix = "auto __cdecl type_name<";
                suffix = ">(void) noexcept";
            #endif
            name.remove_prefix(prefix.size());
            name.remove_suffix(suffix.size());
            return name;
        }
    };
}