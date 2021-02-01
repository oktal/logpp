#include "gtest/gtest.h"

#include "logpp/utils/string.h"

using namespace logpp;

TEST(StringTests, should_parse_size)
{
    struct TestCase
    {
        std::string_view name;

        std::string_view str;
        std::optional<size_t> expected;
    } testCases[] = {
        { 
            "Parse without suffix",
            "1024",
            1024
        },
        { 
            "Parse without suffix negative",
            "-1024",
            std::nullopt
        },
        { 
            "Parse invalid",
            "abcd",
            std::nullopt,
        },
        {
            "Parse with kb suffix",
            "10kb",
            10 * 1024
        },
        {
            "Parse with kb suffix case",
            "10Kb",
            10 * 1024
        },
        {
            "Parse with mb suffix",
            "10mb",
            10 * 1024 * 1024
        },
        {
            "Parse with mb suffix case",
            "10Mb",
            10 * 1024 * 1024
        },
        {
            "Parse with gb suffix",
            "10gb",
            10ULL * 1024 * 1024 * 1024
        },
        {
            "Parse with gb suffix case",
            "10Gb",
            10ULL * 1024 * 1024 * 1024
        },
        {
            "Parse with invalid suffix",
            "10aab",
            std::nullopt,
        },
    };

    for (const auto& test: testCases)
    {
        auto result = string_utils::parseSize(test.str);
        ASSERT_EQ(result, test.expected) << "Test " << test.name << " failed";
    }
}