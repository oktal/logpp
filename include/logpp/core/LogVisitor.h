#pragma once

#include <string_view>

namespace logpp
{
    class LogVisitor
    {
    public:
        virtual void visit(std::string_view key, std::string_view value) = 0;

        virtual void visit(std::string_view key, uint8_t value) = 0;
        virtual void visit(std::string_view key, uint16_t value) = 0;
        virtual void visit(std::string_view key, uint32_t value) = 0;
        virtual void visit(std::string_view key, uint64_t value) = 0;

        virtual void visit(std::string_view key, int8_t value) = 0;
        virtual void visit(std::string_view key, int16_t value) = 0;
        virtual void visit(std::string_view key, int32_t value) = 0;
        virtual void visit(std::string_view key, int64_t value) = 0;

        virtual void visit(std::string_view key, float value) = 0;
        virtual void visit(std::string_view key, double value) = 0;
    };
}