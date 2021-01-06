function(logpp_sample sample_name)
    set(SAMPLE_EXECUTABLE sample_${sample_name})
    set(SAMPLE_SOURCE ${sample_name}.cpp)

    add_executable(${SAMPLE_EXECUTABLE} ${SAMPLE_SOURCE})
    target_link_libraries(${SAMPLE_EXECUTABLE} logpp::logpp)
endfunction()

logpp_sample(BasicLogger)
logpp_sample(AsyncLogger)