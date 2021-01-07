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
            logpp::data("username", userName),
            logpp::data("password", password),
            logpp::data("attempts", 1)
        );

        m_logger->info("User authorized",
            logpp::data("username", userName),
            logpp::data("password", password)
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