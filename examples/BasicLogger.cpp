#include "logpp/core/LoggerFactory.h"
#include "logpp/sinks/LogFmt.h"

#include <iostream>

class Authorizer
{
public:
    Authorizer()
    {
        auto logFmt = std::make_shared<logpp::sink::LogFmt>(std::cout);

        m_logger = logpp::LoggerFactory::getLogger<Authorizer>(logFmt);
    }

    bool authorize(const std::string& userName, const std::string& password)
    {
        m_logger->debug("Authorizing user",
            logpp::structure("username", userName),
            logpp::structure("password", password),
            logpp::structure("attempts", 1)
        );

        m_logger->info("User authorized",
            logpp::structure("username", userName),
            logpp::structure("password", password)
        );

        return true;
    }

private:
    std::shared_ptr<logpp::Logger> m_logger;
};

int main()
{
    Authorizer authorizer;
    authorizer.authorize("admin", "admin");
}