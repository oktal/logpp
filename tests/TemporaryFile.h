#pragma once

#include "logpp/core/config.h"
#include "logpp/sinks/file/RollingOfstream.h"
#include "logpp/utils/file.h"

#include <fmt/format.h>
#include <iostream>

#if defined(LOGPP_PLATFORM_WINDOWS)
#include <windows.h>
#include <fileapi.h>
#include <rpc.h>
#pragma comment(lib, "Rpcrt4.lib")
#endif

namespace logpp
{
    inline std::string createTemporaryFileName(std::string_view prefix, std::string_view suffix)
    {
        // We are not guaranteed that file names generated by this kind of algorithm will be unique
        // However, we are generating a unique temporary directory, so full path of temporary files
        // will be unique in that unique directory
        static size_t idx = 0;
        return fmt::format("{}{}{}", prefix, idx++, suffix);
    }

    inline std::string createTemporaryDirectory()
    {
#if defined(LOGPP_PLATFORM_LINUX)
        char templateName[] = "logpptmpXXXXXX";
        auto res            = mkdtemp(templateName);
        if (!res)
            throw std::runtime_error("failed to create temporary directory");
        return std::string(templateName);
#elif defined(LOGPP_PLATFORM_WINDOWS)
        UUID uuid;
        if (UuidCreate(&uuid) != RPC_S_OK)
            throw std::runtime_error("failed to create temporary directory: could not generate uuid-based unique name");

        unsigned char* str;
        if (UuidToString(&uuid, &str) != RPC_S_OK)
            throw std::runtime_error("failed to create temporary directory: could not generate uuid-based unique name");

        auto dir = std::string("logpptmp-") + std::string(reinterpret_cast<char*>(str));
        RpcStringFree(&str);

        if (!CreateDirectoryA(dir.c_str(), NULL))
            throw std::runtime_error("failed to create temporary directory");

        return dir;
#else
#error "Unsupported platform"
#endif
    }

    class temporary_rolling_filebuf : public rolling_filebuf_base<char>
    {
    public:
        temporary_rolling_filebuf(std::ios_base::openmode openMode, std::string_view prefix, std::string_view suffix)
            : m_directory(createTemporaryDirectory())
            , m_name(createTemporaryFileName(prefix, suffix))
        {
            auto path = fmt::format("{}/{}", m_directory, m_name);
            if (!open(path, openMode))
                throw std::runtime_error(fmt::format("Failed to open file {}", path));
            this->path(path);
            this->mode(openMode);
        }

        std::string_view directory() const
        {
            return m_directory;
        }

        std::string_view name() const
        {
            return m_name;
        }

        bool can_roll() override { return false; }
        void roll() override { }

    private:
        std::string m_directory;
        std::string m_name;
    };

    class temporary_ofstream : public std::ofstream
    {
    public:
        temporary_ofstream(std::ios_base::openmode openMode, std::string_view prefix, std::string_view suffix)
            : m_directory(createTemporaryDirectory())
            , m_name(createTemporaryFileName(prefix, suffix))
            , m_path(fmt::format("{}/{}", m_directory, m_name))
        {
            open(m_path, openMode);
            if (!*this)
                throw std::runtime_error(fmt::format("Failed to open file {}", m_path));
        }

        std::string_view directory() const
        {
            return m_directory;
        }

        std::string_view name() const
        {
            return m_name;
        }

        std::string_view path() const
        {
            return m_path;
        }

    private:
        std::string m_directory;
        std::string m_name;
        std::string m_path;
    };

    struct RemoveDirectoryOnExit
    {
    public:
        explicit RemoveDirectoryOnExit(std::string_view dir)
            : m_dir(dir)
        { }

        ~RemoveDirectoryOnExit()
        {
            // If we fail to remove the directory, print a warning with the path to give
            // a chance to the user to manually clean it.
            if (!file_utils::removeAll(m_dir))
                std::cerr << "WARNING: Failed to remove directory '" << m_dir << "'\n";
        }

    private:
        std::string m_dir;
    };
}
