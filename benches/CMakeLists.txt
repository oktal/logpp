function(logpp_bench bench_name)
    set(BENCH_EXECUTABLE bench_${bench_name})
    set(BENCH_SOURCE ${bench_name}.cpp)

    add_executable(${BENCH_EXECUTABLE} ${BENCH_SOURCE})
    target_link_libraries(${BENCH_EXECUTABLE} ${CONAN_LIBS_BENCHMARK} logpp::logpp)
endfunction()

logpp_bench(LoggerBench)