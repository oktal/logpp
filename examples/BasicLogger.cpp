#include "logpp/core/LoggerFactory.h"
#include "logpp/sinks/LogFmt.h"

#include <iostream>

class Authorizer
{
public:
    struct AuthorizeLogEvent
    {
        logpp::StringOffset userName;
        logpp::StringOffset password;
        logpp::Offset<uint8_t> attempts;

        void format(logpp::LogBufferView buffer, logpp::LogWriter& writer) const
        {
            writer.write("username", buffer, userName);
            writer.write("password", buffer, password);
            writer.write("attempts", buffer, attempts);
        }
    };

    Authorizer()
    {
        auto logFmt = std::make_shared<logpp::sink::LogFmt>(std::cout);

        m_logger = logpp::LoggerFactory::getLogger<Authorizer>(logFmt);
    }

    bool authorize(const std::string& userName, const std::string& password)
    {
        m_logger->debug("Authorizing user", [&](logpp::LogBufferBase& buffer, AuthorizeLogEvent& event) {
            event.userName = buffer.write(userName);
            event.password = buffer.write(password);
        });

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