#pragma once

#include "logpp/core/Offset.h"
#include "logpp/core/StringLiteral.h"

#include <array>
#include <cstring>
#include <memory>

namespace logpp
{

    class LogBufferBase
    {
    public:
        friend class LogBufferView;

        virtual ~LogBufferBase() = default;

        template <typename T>
        Offset<T> write(T value)
        {
            return offsetAt<T>(encode(value));
        }

        template <size_t N>
        StringOffset write(const char (&str)[N])
        {
            return writeString(str, N - 1);
        }

        StringOffset write(std::string_view str);

        StringOffset write(const std::string& str);
        StringOffset write(const char* str);

        StringLiteralOffset write(StringLiteral str);

        template <typename Return, typename... Args>
        FunctionOffset write(Return (*func)(Args...))
        {
            return offsetAt<tag::Function>(encode(func));
        }

        size_t size() const;

    private:
        size_t m_cursor { 0 };

        StringOffset writeString(const char* str, size_t size);
        virtual void reserve(size_t capacity) = 0;

    protected:
        virtual char* dataAt(size_t index) = 0;

        const char* dataAt(size_t index) const;

        template <typename T>
        T* overlayAt(size_t offset)
        {
            return reinterpret_cast<T*>(dataAt(offset));
        }

        template <typename T>
        const T* overlayAt(size_t offset) const
        {
            return reinterpret_cast<const T*>(dataAt(offset));
        }

        template <typename T>
        size_t encode(T value)
        {
            reserve(size() + sizeof(value));
            auto index           = m_cursor;
            *overlayAt<T>(index) = value;
            m_cursor += sizeof(T);
            return index;
        }

        size_t encodeRaw(const char* bytes, size_t size);
        size_t encodeString(const char* str, size_t size);

        void advance(size_t size);
        size_t cursor() const;

        template <typename T>
        Offset<T> offsetAt(size_t index) const
        {
            return Offset<T> { static_cast<uint16_t>(index) };
        }
    };

    template <size_t N>
    class LogBuffer : public LogBufferBase
    {
    public:
        LogBuffer()
            : m_data(&m_inlineData[0])
        { }

        ~LogBuffer()
        {
            if (!isSmall())
            {
                std::free(m_data);
                m_data = nullptr;
            }
        }

        LogBuffer(const LogBuffer& other)
            : m_data(&m_inlineData[0])
        {
            *this = other;
        }

        LogBuffer(LogBuffer&& other) noexcept
            : m_data(&m_inlineData[0])
        {
            *this = std::move(other);
        }

        LogBuffer& operator=(const LogBuffer& other)
        {
            reserve(other.m_capacity);
            std::memcpy(m_data, other.m_data, other.size());

            m_capacity = other.m_capacity;
            advance(other.cursor());

            return *this;
        }

        LogBuffer& operator=(LogBuffer&& other) noexcept
        {
            // If the other is not small, let's steal its buffer
            if (!other.isSmall())
            {
                // Cleanup our buffer if we were not small
                if (!isSmall())
                    std::free(m_data);

                m_data       = other.m_data;
                other.m_data = nullptr;
            }
            // The other is small, which means that we will also be small, so memcpy the bytes
            // over
            else
            {
                // Cleanup our buffer if we were not small
                if (!isSmall())
                    std::free(m_data);

                m_data = &m_inlineData[0];
                std::memcpy(m_data, other.m_data, other.size());
            }

            advance(other.cursor());
            m_capacity = other.m_capacity;

            return *this;
        }

    private:
        using InlineStorage = std::array<char, N>;

        InlineStorage m_inlineData {};
        char* m_data { nullptr };

        size_t m_capacity { N };

        bool isSmall() const
        {
            return m_data == &m_inlineData[0];
        }

        void reserve(size_t capacity) override
        {
            if (capacity <= m_capacity)
                return;

            auto newCapacity = std::max(m_capacity * 2, capacity);

            auto* data = static_cast<char*>(malloc(newCapacity * sizeof(char)));
            if (data == nullptr)
                throw std::bad_alloc();

            if (isSmall())
            {
                std::memcpy(data, m_inlineData.data(), m_inlineData.size());
            }
            else
            {
                std::memcpy(data, m_data, this->cursor());
                free(m_data);
            }

            m_data     = data;
            m_capacity = newCapacity;
        }

    protected:
        char* dataAt(size_t index) override
        {
            return m_data + index;
        }
    };
}