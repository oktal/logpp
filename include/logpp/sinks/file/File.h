#pragma once

#include <fstream>
#include <memory>
#include <string_view>

namespace logpp::sink
{
    class File
    {
    public:
        virtual ~File() = default;

        std::string_view path() const
        {
            return m_path;
        }

        virtual bool isOpen() const = 0;
        virtual bool close()        = 0;

        size_t write(std::string_view str)
        {
            return write(str.data(), str.size());
        }

        virtual size_t write(const char* data, size_t size) = 0;
        virtual size_t write(const char c)                  = 0;
        virtual void flush()                                = 0;

        virtual size_t size() const = 0;

    protected:
        std::string m_path;
    };

}
