#include "logpp/core/config.h"

#include <regex>
#include <string>
#include <string_view>

#include <cstdlib>

namespace logpp
{
    namespace env_utils
    {
        inline std::string expandEnvironmentVariables(std::string_view str)
        {
            static std::regex varRegex("\\$\\{([^}]+)\\}");

            std::string res(str);
            std::smatch match;
            while (std::regex_search(res, match, varRegex))
            {
                const auto* value    = std::getenv(match[1].str().c_str());
                std::string_view var = value == nullptr ? "" : value;
                res.replace(match[0].first, match[0].second, var);
            }

            return res;
        }

#if defined(LOGPP_COMPILER_MSVC)
        inline int setenv(const char* name, const char* value, int overwrite)
        {
            int err = 0;
            if (!overwrite)
            {
                size_t size = 0;
                err         = getenv_s(&size, NULL, 0, name);
                if (err != 0 || size > 0)
                    return err;
            }
            return _putenv_s(name, value);
        }
#else
        inline int setenv(const char* name, const char* value, int overwrite)
        {
            return ::setenv(name, value, overwrite);
        }
#endif
    }
}
