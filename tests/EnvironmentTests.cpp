#include "logpp/utils/env.h"

#include "gtest/gtest.h"

using namespace logpp;

TEST(EnvironmentTests, should_expand_string_with_no_variable_to_same_value)
{
    std::string str("This is a normal string");
    ASSERT_EQ(env_utils::expandEnvironmentVariables(str), str);
}

TEST(EnvironmentTests, should_expand_unknown_variable_to_empty_string)
{
    std::string str1("${LOGPP_NOT_A_VAR}");
    ASSERT_EQ(env_utils::expandEnvironmentVariables(str1), "");

    std::string str2("${LOGPP_NOT_A_VAR}.log");
    ASSERT_EQ(env_utils::expandEnvironmentVariables(str2), ".log");
}

TEST(EnvironmentTests, should_expand_variable)
{
    env_utils::setenv("LOGPP_DEFAULT_LOGGER", "logpp", 1);

    std::string str1("${LOGPP_DEFAULT_LOGGER}");
    ASSERT_EQ(env_utils::expandEnvironmentVariables(str1), "logpp");

    std::string str2("${LOGPP_DEFAULT_LOGGER}.log");
    ASSERT_EQ(env_utils::expandEnvironmentVariables(str2), "logpp.log");
}