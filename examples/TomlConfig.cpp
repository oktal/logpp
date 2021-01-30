#include "logpp/config/TomlConfigurator.h"

#include "logpp/logpp.h"

#include <iostream>

int main(int argc, const char* argv[])
{
    std::string file = "logpp.toml";
    if (argc == 2)
        file = argv[1];

    std::cout << "Configuring logger with " << file << std::endl;

    auto err = logpp::TomlConfigurator::configureFile(file);
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