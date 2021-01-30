include(GoogleTest)

function(logpp_test test_name)
    set(TEST_EXECUTABLE test_${test_name})
    set(TEST_SOURCE ${test_name}.cpp)

    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCE})
    target_link_libraries(${TEST_EXECUTABLE} PRIVATE ${CONAN_LIBS_GTEST} logpp::logpp)
    gtest_discover_tests(${TEST_EXECUTABLE})
endfunction()

logpp_test(ArchiveStrategyTests)
logpp_test(EnvironmentTests)
logpp_test(LogBufferTests)
logpp_test(LoggerRegistryTests)
logpp_test(PatternFormatterTests)
logpp_test(RollingStrategyTests)
logpp_test(StringTests)
logpp_test(TomlConfiguratorTests)