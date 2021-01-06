include(GoogleTest)

function(logpp_test test_name)
    set(TEST_EXECUTABLE test_${test_name})
    set(TEST_SOURCE ${test_name}.cpp)

    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCE})
    target_link_libraries(${TEST_EXECUTABLE} ${CONAN_LIBS_GTEST} logpp::logpp)
    gtest_discover_tests(${TEST_EXECUTABLE})
endfunction()

logpp_test(LogBufferTests)