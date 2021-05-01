function (logpp_fuzz fuzz_name)
    set(FUZZ_EXECUTABLE fuzz_${fuzz_name})
    set(FUZZ_SOURCE ${fuzz_name}.cpp)

    add_executable(${FUZZ_EXECUTABLE} ${FUZZ_SOURCE})
    target_compile_options(${FUZZ_EXECUTABLE} PRIVATE -fsanitize=address,fuzzer)
    target_link_libraries( ${FUZZ_EXECUTABLE} PRIVATE -fsanitize=address,fuzzer logpp::logpp)
endfunction()

logpp_fuzz(AsyncSinkFuzz)
logpp_fuzz(EventLogBufferFuzz)
