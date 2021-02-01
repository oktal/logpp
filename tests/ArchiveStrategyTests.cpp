#include "logpp/sinks/file/ArchiveStrategy.h"

#include "logpp/utils/date.h"
#include "logpp/utils/file.h"

#include "TemporaryFile.h"

#include "gtest/gtest.h"

#include <fstream>

using namespace logpp;
using namespace logpp::sink;

using namespace date;
using namespace std::chrono;
using namespace std::string_view_literals;

struct RemoveDirectoryOnExit
{
public:
    explicit RemoveDirectoryOnExit(std::string_view dir)
        : m_dir(dir)
    {}

    ~RemoveDirectoryOnExit()
    {
        // If we fail to remove the directory, print a warning with the path to give
        // a chance to the user to manually clean it.
        if (!file_utils::removeAll(m_dir))
            std::cerr << "WARNING: Failed to remove directory '" << m_dir << "'\n";
    }

private:
    std::string m_dir;
};

TEST(ArchiveStrategyTests, should_archive_with_incrementing_number)
{
    auto ymd = jan/23/2021;
    auto now = date::sys_days{ymd};

    IncrementalArchiveStrategy strategy;
    auto file = std::make_unique<TemporaryFile>(std::ios_base::out, "logpptestfile", ".log");
    auto basePath = file->path();

    RemoveDirectoryOnExit rmDir(file->directory());

    file->write("File0"sv);
    file->flush();

    std::unique_ptr<File> newFile(strategy.apply(now, file.get()));
    newFile->write("File1"sv);
    newFile->flush();

    std::unique_ptr<File> newFile2(strategy.apply(now, newFile.get()));
    newFile2->write("File2"sv);
    newFile2->flush();

    std::unique_ptr<File> newFile3(strategy.apply(now, newFile2.get()));
    newFile3->write("File3"sv);
    newFile3->flush();

    auto check = [](std::string_view baseFileName, std::optional<size_t> n, std::function<void (const std::string&)> assertFunc)
    {
        auto fileName = std::string(baseFileName);
        if (n)
        {
            fileName.push_back('.');
            fileName.append(std::to_string(*n));
        }

        ASSERT_TRUE(file_utils::exists(fileName)) << "File " << fileName << " does not exist";
        std::ifstream in(fileName);
        ASSERT_TRUE(in) << "Failed to open file: " << fileName;
        std::string str((std::istreambuf_iterator<char>(in)),
                        (std::istreambuf_iterator<char>()));
        assertFunc(str);
    };

    check(basePath, std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File3");
    });

    check(basePath, 0, [](const std::string& s) {
        ASSERT_EQ(s, "File2");
    });

    check(basePath, 1, [](const std::string& s) {
        ASSERT_EQ(s, "File1");
    });

    check(basePath, 2, [](const std::string& s) {
        ASSERT_EQ(s, "File0");
    });
}

TEST(ArchiveStrategyTests, should_apply_timestamp)
{
    TimestampArchiveStrategy<SystemClock> strategy("%Y%m%d");

    auto ymd = jan/23/2021;
    auto now = date::sys_days{ymd} + hours{9} + minutes{10} + seconds{35};

    auto file = std::make_unique<TemporaryFile>(std::ios_base::out, "logpptestfile", ".log");
    auto basePath = file->path();

    RemoveDirectoryOnExit rmDir(file->directory());

    file->write("File0"sv);
    file->flush();

    std::unique_ptr<File> newFile(strategy.apply(now, file.get()));
    newFile->write("File1"sv);
    newFile->flush();

    auto check = [](std::string_view baseFileName, std::optional<std::string> suffix, std::function<void (const std::string&)> assertFunc)
    {
        auto fileName = std::string(baseFileName);
        if (suffix)
        {
            fileName.push_back('.');
            fileName.append(*suffix);
        }

        ASSERT_TRUE(file_utils::exists(fileName)) << "File " << fileName << " does not exist";
        std::ifstream in(fileName);
        ASSERT_TRUE(in) << "Failed to open file: " << fileName;
        std::string str((std::istreambuf_iterator<char>(in)),
                        (std::istreambuf_iterator<char>()));
        assertFunc(str);
    };

    check(basePath, std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File1");
    });

    check(basePath, "20210123", [](const std::string& s) {
        ASSERT_EQ(s, "File0");
    });
}

TEST(ArchiveStrategyTests, should_apply_incremental_if_already_exists)
{
    TimestampArchiveStrategy<SystemClock> strategy("%Y%m%d");

    auto ymd = jan/23/2021;
    auto now = date::sys_days{ymd};

    auto file = std::make_unique<TemporaryFile>(std::ios_base::out, "logpptestfile", ".log");
    auto basePath = file->path();

    RemoveDirectoryOnExit rmDir(file->directory());

    file->write("File0"sv);
    file->flush();

    std::unique_ptr<File> newFile(strategy.apply(now, file.get()));
    newFile->write("File1"sv);
    newFile->flush();

    std::unique_ptr<File> newFile2(strategy.apply(now, newFile.get()));
    newFile2->write("File2"sv);
    newFile2->flush();

    std::unique_ptr<File> newFile3(strategy.apply(now, newFile2.get()));
    newFile3->write("File3"sv);
    newFile3->flush();

    auto check = [](std::string_view baseFileName, std::optional<std::string> suffix, std::optional<size_t> n, std::function<void (const std::string&)> assertFunc)
    {
        auto fileName = std::string(baseFileName);
        if (suffix)
        {
            fileName.push_back('.');
            fileName.append(*suffix);
        }
        if (n)
        {
            fileName.push_back('.');
            fileName.append(std::to_string(*n));
        }

        ASSERT_TRUE(file_utils::exists(fileName)) << "File " << fileName << " does not exist";
        std::ifstream in(fileName);
        ASSERT_TRUE(in) << "Failed to open file: " << fileName;
        std::string str((std::istreambuf_iterator<char>(in)),
                        (std::istreambuf_iterator<char>()));
        assertFunc(str);
    };

    check(basePath, std::nullopt, std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File3");
    });

    check(basePath, "20210123", 0, [](const std::string& s) {
        ASSERT_EQ(s, "File1");
    });

    check(basePath, "20210123", 1, [](const std::string& s) {
        ASSERT_EQ(s, "File0");
    });

    check(basePath, "20210123", std::nullopt, [](const std::string& s) {
        ASSERT_EQ(s, "File2");
    });
}