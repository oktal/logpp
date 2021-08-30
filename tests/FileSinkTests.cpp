#include "gtest/gtest.h"

#include "logpp/sinks/file/FileSink.h"
#include "logpp/utils/file.h"

#include "TemporaryFile.h"

using namespace logpp;
using namespace logpp::sink;

TEST(FileSink, should_raise_configuration_error_when_missing_file_option)
{
    auto sink = std::make_shared<FileSink>();

    Options options;
    ASSERT_THROW(sink->activateOptions(options), ConfigurationError);
}

TEST(FileSink, should_create_directory_if_does_not_exist)
{
    auto sink = std::make_shared<FileSink>();

    auto directory = createTemporaryDirectory();

    RemoveDirectoryOnExit rmDir(directory);

    auto filePath = fmt::format("{}/test.log", directory);
    ASSERT_FALSE(file_utils::exists(filePath));

    Options options;
    options.add("file", filePath);

    ASSERT_NO_THROW(sink->activateOptions(options));
    ASSERT_TRUE(file_utils::exists(filePath));
}
