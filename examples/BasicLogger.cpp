#include "logpp/logpp.h"

#include "logpp/core/Logger.h"
#include "logpp/sinks/ColoredConsole.h"

enum class AccessRight {
    Read,
    Write,
    All
};

std::string_view accessRightString(AccessRight access)
{
    using namespace std::string_view_literals;

    switch (access)
    {
    case AccessRight::Read:
        return "Read"sv;
    case AccessRight::Write:
        return "Write"sv;
    case AccessRight::All:
        return "All"sv;
    }

    return ""sv;
}

bool authorizeUser(const std::string& userName, const std::string& password)
{
    logpp::debug("Authorizing user",
                 logpp::field("username", userName),
                 logpp::field("password", password));

    return true;
}

bool grantUser(const std::string& userName, AccessRight access)
{
    if (userName == "admin")
        logpp::warn("Attempting to grant access to admin user");

    logpp::debug(logpp::format("Granting access to user {}", userName),
                 logpp::field("access_right", accessRightString(access)));

    return true;
}

int main(int argc, const char* argv[])
{
    // Set the global logger level to Debug
    logpp::setLevel(logpp::LogLevel::Debug);
    auto formatter = std::make_shared<logpp::LogFmtFormatter>();
    auto consoleOut = std::make_shared<logpp::sink::ColoredOutputConsole>(formatter);
    logpp::defaultLogger()->setSink(consoleOut);

    // Uncomment this line to log lines in logfmt format
    // logpp::setFormatter<logpp::LogFmtFormatter>();

    // Uncomment this line to display source location. Note that only messages logged from LOGPP_*
    // macros (see below) will have a location.
    // logpp::setFormatter<logpp::PatternFormatter>("%Y-%m-%d %H:%M:%S [%l] (%n) %p:%o %v%f[ - ]");

    if (argc < 3)
    {
        logpp::error(logpp::format("Usage: {} [username] [password]", argv[0]));
        return 0;
    }

    // You can use LOGPP_* macros as plain replacement of logging functions.
    // Using macros will also add the source location from which the log message originated from.
    // Set LogLevel above to Trace to show this message.
    LOGPP_TRACE(LOGPP_FORMAT("Running {}", argv[0]),
                LOGPP_FIELD("user_name", argv[1]),
                LOGPP_FIELD("passowrd", argv[2]));

    if (authorizeUser(argv[1], argv[2]))
    {
        if (grantUser(argv[1], AccessRight::Read))
        {
            logpp::info("User had been granted",
                        logpp::field("access_right", accessRightString(AccessRight::Read)));
        }
    }
}
