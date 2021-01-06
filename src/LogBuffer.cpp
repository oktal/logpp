#include "logpp/core/LogBuffer.h"

namespace logpp
{
    StringOffset LogBufferBase::write(std::string_view str)
    {
        return writeString(str.data(), str.size());
    }

    StringOffset LogBufferBase::write(const std::string& str)
    {
        return writeString(str.data(), str.size());
    }

    StringOffset LogBufferBase::write(const char* str)
    {
        return writeString(str, std::strlen(str));
    }

    size_t LogBufferBase::size() const
    {
        return m_cursor;
    }

    StringOffset LogBufferBase::writeString(const char* str, size_t size)
    {
        return offsetAt< tag::String >(encodeString(str, size));
    }

    const char* LogBufferBase::dataAt(size_t index) const
    {
        return const_cast< LogBufferBase* >(this)->dataAt(index);
    }

    size_t LogBufferBase::encodeRaw(const char* bytes, size_t size)
    {
        reserve(this->size() + size);
        auto index = m_cursor;
        std::memcpy(dataAt(index), bytes, size);
        m_cursor += size;
        return index;
    }

    size_t LogBufferBase::encodeString(const char* str, size_t size)
    {
        auto index = encode(static_cast< uint16_t >(size));
        encodeRaw(str, size);

        return index;
    }

    void LogBufferBase::advance(size_t size)
    {
        m_cursor += size;
    }

    size_t LogBufferBase::cursor() const
    {
        return m_cursor;
    }

}