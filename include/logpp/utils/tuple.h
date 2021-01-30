#pragma once

#include <utility>
#include <cstddef>

namespace logpp
{
    namespace tuple_utils
    {
        namespace impl
        {
            template<size_t... Indexes, typename Tuple, typename Visitor>
            void visit(std::index_sequence<Indexes...>, Tuple&& tuple, Visitor&& visitor)
            {
                (visitor(std::get<Indexes>(tuple)),...);
            }
        }

        template<typename Visitor, typename Tuple>
        void visit(Tuple&& tuple, Visitor&& visitor)
        {
            using TupleT = std::decay_t<Tuple>;
            static constexpr auto Size = std::tuple_size_v<TupleT>;

            impl::visit(std::make_index_sequence<Size>{}, std::forward<Tuple>(tuple), std::forward<Visitor>(visitor));
        }
    }
}