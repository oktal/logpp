#pragma once

namespace logpp
{
    struct StringLiteral
    {
        explicit StringLiteral(const char* value)
            : value(value)
        { }

        const char* value;
    };

}