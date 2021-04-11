#pragma once

#include <cstddef>
#include <string_view>

namespace logpp
{
    struct SourceLocation
    {
        std::string_view file;
        size_t line;
    };
}
