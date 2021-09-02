#pragma once

#include "logpp/core/config.h"

#if defined(LOGPP_CPP17_FILESYSTEM)
#include <filesystem>
#endif

#include <cstdlib>
#include <string_view>

namespace logpp::file_utils
{

#if defined(LOGPP_CPP17_FILESYSTEM)
    inline bool exists(std::string_view path)
    {
        return std::filesystem::exists(path);
    }

    inline bool move(std::string_view from, std::string_view to)
    {
        try
        {
            std::filesystem::rename(from, to);
            return true;
        }
        catch (const std::filesystem::filesystem_error&)
        {
            return false;
        }
    }

    inline bool remove(std::string_view path)
    {
        return std::filesystem::remove(path);
    }

    inline size_t removeAll(std::string_view path)
    {
        return static_cast<size_t>(std::filesystem::remove_all(path));
    }

    inline std::string directory(std::string_view path)
    {
        std::filesystem::path p { path };
        return p.parent_path().string();
    }

    inline std::string fileName(std::string_view path)
    {
        std::filesystem::path p { path };
        return p.filename().string();
    }

    inline bool createDirectories(std::string_view path)
    {
        std::filesystem::path p(path);
        if (p.has_filename())
        {
            auto parent = p.parent_path();
            if (parent.empty())
                return true;
            return std::filesystem::create_directories(parent);
        }

        return std::filesystem::create_directories(path);
    }

#else
#error "Unknown compiler version for filesystem utilities"
#endif


}
