#include "logpp/config/TomlConfigurator.h"

#include "logpp/logpp.h"

#include <iostream>

int main(int argc, const char* argv[])
{
    auto err = logpp::TomlConfigurator::configureFile("logpp.toml");
    if (err)
    {
        std::cerr << "Error configuring logger: " << *err << std::endl;
        return 0;
    }

    auto logger = logpp::getLogger("main");
    logger->info("This is an informational message",
        logpp::field("exe_name", argv[0])
    );
}