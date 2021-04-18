#pragma once

#include "logpp/utils/detect.h"

#include <string_view>

namespace logpp
{
    class LogFieldVisitor
    {
    public:
        virtual void visitStart(size_t count) = 0;

        virtual void visit(std::string_view key, std::string_view value) = 0;

        virtual void visit(std::string_view key, uint8_t value)  = 0;
        virtual void visit(std::string_view key, uint16_t value) = 0;
        virtual void visit(std::string_view key, uint32_t value) = 0;
        virtual void visit(std::string_view key, uint64_t value) = 0;

        virtual void visit(std::string_view key, int8_t value)  = 0;
        virtual void visit(std::string_view key, int16_t value) = 0;
        virtual void visit(std::string_view key, int32_t value) = 0;
        virtual void visit(std::string_view key, int64_t value) = 0;

        virtual void visit(std::string_view key, bool value)   = 0;
        virtual void visit(std::string_view key, float value)  = 0;
        virtual void visit(std::string_view key, double value) = 0;

        virtual void visitEnd() = 0;
    };

    namespace details
    {
        template <typename T>
        auto visit() -> decltype(std::declval<LogFieldVisitor>().visit(std::declval<std::string_view>(), std::declval<T>()));

        template <typename T, typename Enable = void>
        struct HasVisit : std::false_type
        { };

        template <typename T>
        struct HasVisit<T, std::void_t<decltype(visit<T>())>> : std::true_type
        { };

        template <>
        struct HasVisit<std::string, void> : std::true_type
        { };
    }

    template <typename T>
    static constexpr bool IsVisitable = details::HasVisit<T>::value;

#define VISIT_STATIC_CHECK(Type) \
    static_assert(IsVisitable<Type>, "Type is not visitable")

    VISIT_STATIC_CHECK(std::string_view);
    VISIT_STATIC_CHECK(const char*);
    VISIT_STATIC_CHECK(std::string);

    VISIT_STATIC_CHECK(uint8_t);
    VISIT_STATIC_CHECK(uint16_t);
    VISIT_STATIC_CHECK(uint32_t);
    VISIT_STATIC_CHECK(uint64_t);

    VISIT_STATIC_CHECK(int8_t);
    VISIT_STATIC_CHECK(int16_t);
    VISIT_STATIC_CHECK(int32_t);
    VISIT_STATIC_CHECK(int64_t);

    VISIT_STATIC_CHECK(bool);
    VISIT_STATIC_CHECK(float);
    VISIT_STATIC_CHECK(double);

#undef VISIT_STATIC_CHECK
}
