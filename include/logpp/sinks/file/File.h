#pragma once

#include <fstream>
#include <memory>
#include <string_view>

namespace logpp::sink
{
    class File
    {
    public:
        File()
            : m_path("")
            , m_fs(nullptr)
            , m_size(0ULL)
        {}

        ~File()
        {
            if (isOpen())
                close();
        }

        explicit File(std::string_view filePath, std::ios_base::openmode openMode)
        {
            open(filePath, openMode);
        }

        std::string_view path() const
        {
            return m_path;
        }

        bool open(std::string_view filePath, std::ios_base::openmode openMode)
        {
            auto fs = std::make_unique<std::ofstream>(filePath.data(), openMode);
            if (fs->bad())
                return false;

            std::swap(m_fs, fs);
            m_path = filePath;
            return true;
        }

        bool isOpen() const
        {
            return m_fs && m_fs->is_open();
        }

        bool close()
        {
            if (!m_fs)
                return false;

            m_fs->close();
            m_fs.reset();

            return true;
        }

        size_t write(std::string_view str)
        {
            return write(str.data(), str.size());
        }

        virtual size_t write(const char* data, size_t size)
        {
            if (!m_fs)
                return 0ULL; 

            m_fs->write(data, size);
            m_size += size;
            return size;
        }

        virtual size_t write(const char c)
        {
            if (!m_fs)
                return 0ULL;

            m_fs->put(c);
            return 1ULL;
        }

        virtual size_t size() const
        {
            return m_size;
        }

        void flush()
        {
            if (!m_fs)
                return;

            m_fs->flush();
        }

    private:
        std::string m_path;
        std::unique_ptr<std::ofstream> m_fs;
        size_t m_size;
    };

}