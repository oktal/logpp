function(logpp_sample sample_name)
    set(SAMPLE_EXECUTABLE sample_${sample_name})
    set(SAMPLE_SOURCE ${sample_name}.cpp)

    add_executable(${SAMPLE_EXECUTABLE} ${SAMPLE_SOURCE})
    target_link_libraries(${SAMPLE_EXECUTABLE} logpp::logpp)
endfunction()

function(logpp_copy_file)
    set(prefix LOGPP_COPY)
    set(singleValues TARGET FILE)
    set(multiValues)

    include(CMakeParseArguments)
    cmake_parse_arguments(
        ${prefix}
        ""
        "${singleValues}"
        "${multiValues}"
        ${ARGN}
    )

    get_target_property(LOGPP_TARGET_OUTPUT_DIRECTORY ${LOGPP_COPY_TARGET} RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(
        TARGET ${LOGPP_COPY_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/${LOGPP_COPY_FILE}
            ${LOGPP_TARGET_OUTPUT_DIRECTORY}/${LOGPP_COPY_FILE}
    )

endfunction()

logpp_sample(AsyncLogger)
logpp_sample(BasicLogger)
logpp_sample(CustomPatternFormatterFlag)
logpp_sample(TomlConfig)
logpp_sample(TomlCustomSinkConfig)

logpp_copy_file(
    TARGET sample_TomlConfig
    FILE logpp.toml
)
logpp_copy_file(
    TARGET sample_TomlConfig
    FILE logpp.file.custom.toml
)
