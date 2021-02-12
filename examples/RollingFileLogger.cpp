#include "logpp/config/TomlConfigurator.h"
#include "logpp/logpp.h"

#include <thread>

using namespace logpp;

int main()
{
    auto err = TomlConfigurator::configureFile("logpp.file.toml");
    if (err)
    {
        std::cerr << *err << std::endl;
        return 1;
    }

    size_t i = 0;

    auto logger = logpp::getLogger("main");

    for (;;)
    {
        logger->info("This is a log message", logpp::field("i", i));
        std::this_thread::sleep_for(std::chrono::minutes(1));

        ++i;
    }
}