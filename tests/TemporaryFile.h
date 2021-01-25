#pragma once

#include "logpp/sinks/file/FileSink.h"

#include <fmt/format.h>

namespace logpp
{

    class TemporaryFile : public sink::File
    {
    public:
        TemporaryFile(std::ios_base::openmode openMode, std::string_view prefix, std::string_view suffix)
            : m_directory(createTemporaryDirectory())
            , m_name(createTemporaryFileName(prefix, suffix))
        {
            auto path = fmt::format("{}/{}", m_directory, m_name);
            open(path, openMode);
        }

        std::string_view directory() const
        {
            return m_directory;
        }

        std::string_view name() const
        {
            return m_name;
        }

    private:
        std::string m_directory;
        std::string m_name;

        static std::string createTemporaryFileName(std::string_view prefix, std::string_view suffix)
        {
            // We are not guaranteed that file names generated by this kind of algorithm will be unique
            // However, we are generating a unique temporary directory, so full path of temporary files
            // will be unique in that unique directory
            static size_t idx = 0;
            return fmt::format("{}-{}{}", prefix, idx++, suffix);
        }

    #if defined(LOGPP_PLATFORM_LINUX) 
        static std::string createTemporaryDirectory()
        {
            char templateName[] = "logpptmpXXXXXX";
            auto res = mkdtemp(templateName);
            if (!res)
                throw std::runtime_error("failed to create temporary directory");
            return std::string(templateName);
        }
    #endif

    };

}