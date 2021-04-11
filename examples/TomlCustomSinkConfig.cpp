#include "logpp/config/TomlConfigurator.h"
#include "logpp/logpp.h"
#include "logpp/sinks/file/FileSink.h"

#include <iostream>

// Let's create a FileSink that will add a header to the current file
// We first inherit from the FileSink to overwrite the onAfterOpened
// function that will be called right after opening the file and is
// empty by default.
class HeaderFileSink : public logpp::sink::FileSink
{
public:
    // We need to name our sink for registration inside the Registry.
    static constexpr std::string_view Name = "HeaderFileSink";

private:
    // Here is our "header" logic.
    virtual void onAfterOpened(const std::unique_ptr<logpp::sink::File>& file) override
    {
        file->write("ApplicationId: Sample.Custom.Sink\n");
    }
};

int main(int argc, const char* argv[])
{
    auto& registry = logpp::LoggerRegistry::defaultRegistry();

    // We need to register our sink to the Registry if we want to refer to it
    // from the toml configuration file, e.g
    // [sinks.file]
    //     type = "HeaderFileSink"
    //     options = { format = "logfmt", file = "sample.file.custom.log" }
    registry.registerSinkFactory<HeaderFileSink>();

    // Which file do we want to configure from ?
    std::string file = "logpp.file.custom.toml";
    if (argc == 2)
        file = argv[1];

    std::cout << "Configuring logger with " << file << std::endl;

    // Let's configure our registry from a TOML configuration file.
    auto err = logpp::TomlConfigurator::configureFile(file);
    if (err)
    {
        std::cerr << "Error configuring logger: " << *err << std::endl;
        return 0;
    }

    // Get a logger.
    auto logger = logpp::getLogger("main");

    // Log something.
    logger->info("This is an informational message",
                 logpp::field("exe_name", argv[0]));
}
