#pragma once

#include <algorithm>
#include <cctype>
#include <string_view>

#include <cstdlib>

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
            char *end;
            auto val = std::strtol(str.data(), &end, 10);
            if (val < 0)
                return std::nullopt;
            else if (*end == '\0')
                return val;

            const auto suffixSize = str.end() - end;
            std::string_view suffix(end, suffixSize);

            if (iequals(suffix, "kb"))
                return val * 1024;
            else if (iequals(suffix, "mb"))
                return val * 1024 * 1024;
            else if (iequals(suffix, "gb"))
                return val * 1024 * 1024 * 1024;

            return std::nullopt;
        }
    }
}