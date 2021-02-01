#pragma once

#include <algorithm>
#include <optional>
#include <utility>
#include <string_view>

#include <cstdlib>
#include <cctype>

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

        inline std::optional<size_t> parseSize(std::string_view str)
        {
            char *endptr;
            auto val = std::strtoll(str.data(), &endptr, 10);
            if (val < 0)
                return std::nullopt;
            else if (*endptr == '\0')
                return val;

            auto result = static_cast<size_t>(val);

            const auto prefixSize = endptr - str.data();
            std::string_view suffix(str);
            suffix.remove_prefix(prefixSize);

            if (iequals(suffix, "kb"))
                return result * 1024;
            else if (iequals(suffix, "mb"))
                return result * 1024 * 1024;
            else if (iequals(suffix, "gb"))
                return result * 1024 * 1024 * 1024;

            return std::nullopt;
        }
    }
}