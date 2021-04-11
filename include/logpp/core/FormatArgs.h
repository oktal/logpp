#pragma once

#include <string_view>
#include <tuple>

#include <fmt/format.h>

namespace logpp
{
    template <typename... Args>
    struct FormatArgsHolder
    {
        std::string_view formatStr;
        std::tuple<Args...> args;

        void formatTo(fmt::memory_buffer& buffer) const
        {
            formatToImpl(buffer, std::make_index_sequence<sizeof...(Args)> {});
        }

    private:
        template <size_t... Indexes>
        void formatToImpl(fmt::memory_buffer& buffer, std::index_sequence<Indexes...>) const
        {
            fmt::format_to(buffer, formatStr, std::get<Indexes>(args)...);
        }
    };
}