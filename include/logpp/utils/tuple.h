#pragma once

#include <utility>
#include <cstddef>

namespace logpp
{
    namespace tuple_utils
    {
        template<size_t... Indexes, typename Tuple, typename Visitor>
        void visit(std::index_sequence<Indexes...>, Tuple&& tuple, Visitor&& visitor)
        {
            (visitor(std::get<Indexes>(tuple)),...);
        }

        template<typename Tuple>
        constexpr size_t size(const Tuple&)
        {
            return std::tuple_size_v<Tuple>;
        }

        template<typename Visitor, typename Tuple>
        void visit(Tuple&& tuple, Visitor&& visitor)
        {
            visit(std::make_index_sequence<size(tuple)>{}, std::forward<Tuple>(tuple), std::forward<Visitor>(visitor));
        }
    }
}