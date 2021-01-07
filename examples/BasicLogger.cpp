#include "logpp/logpp.h"

#include "logpp/core/Logger.h"
#include "logpp/sinks/LogFmt.h"

bool authorizeUser(const std::string& userName, const std::string& password)
{
    logpp::debug("Authorizing user",
        logpp::data("username", userName),
        logpp::data("password", password)
    );

    return true;
}

int main(int argc, const char *argv[])
{
    if (argc < 3)
        return 0;

    authorizeUser(argv[1], argv[2]);
}