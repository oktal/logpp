function(logpp_sample sample_name)
    set(SAMPLE_EXECUTABLE sample_${sample_name})
    set(SAMPLE_SOURCE ${sample_name}.cpp)

    add_executable(${SAMPLE_EXECUTABLE} ${SAMPLE_SOURCE})
    target_link_libraries(${SAMPLE_EXECUTABLE} logpp::logpp)
endfunction()

function(logpp_copy_file)
    set(prefix LOGPP_COPY)
    set(options VERBOSE)
    set(singleValues TARGET FILE)
    set(multiValues)

    include(CMakeParseArguments)
    cmake_parse_arguments(
        ${prefix}
        "${options}"
        "${singleValues}"
        "${multiValues}"
        ${ARGN}
    )

    set(LOGPP_COPY_TARGET sample_${LOGPP_COPY_TARGET})

    set(LOGPP_TARGET_FILE_DIR $<TARGET_FILE_DIR:${LOGPP_COPY_TARGET}>)

    if (LOGPP_COPY_VERBOSE)
        add_custom_command(
            TARGET ${LOGPP_COPY_TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan --bold
            "[${LOGPP_COPY_TARGET}] Copying file ${LOGPP_COPY_FILE} to ${LOGPP_TARGET_FILE_DIR} ..."
        )
    endif()


    add_custom_command(
        TARGET ${LOGPP_COPY_TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/${LOGPP_COPY_FILE}
            ${LOGPP_TARGET_FILE_DIR}/${LOGPP_COPY_FILE}
    )

endfunction()

logpp_sample(AsyncLogger)
logpp_sample(BasicLogger)
logpp_sample(CustomPatternFormatterFlag)
logpp_sample(TomlConfig)
logpp_sample(TomlCustomSinkConfig)

logpp_copy_file(
    TARGET TomlConfig
    FILE logpp.toml
    VERBOSE
)

logpp_copy_file(
    TARGET TomlCustomSinkConfig
    FILE logpp.file.custom.toml
    VERBOSE
)
