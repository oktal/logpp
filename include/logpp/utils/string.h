#pragma once

#include <algorithm>
#include <cctype>
#include <string_view>

namespace logpp
{
    namespace string_utils
    {
        inline bool iequals(std::string_view lhs, std::string_view rhs)
        {
            return std::equal(
                std::begin(lhs), std::end(lhs), std::begin(rhs),
                [](char c1, char c2) {
                    return std::tolower(c1) == std::tolower(c2);
                }
            );
        }
    }
}