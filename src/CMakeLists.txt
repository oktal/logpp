set(SOURCE_FILES
    AsyncQueuePoller.cpp
    LogBuffer.cpp
    LogBufferView.cpp
    LoggerRegistry.cpp
    LogFmtFormatter.cpp
    PatternFormatter.cpp
    SpinWait.cpp
    TomlConfigurator.cpp
)

add_library(logpp ${SOURCE_FILES})
add_library(logpp::logpp ALIAS logpp)

include(ProcessorCount)
ProcessorCount(LOGPP_CORES)
if(NOT LOGPP_CORES EQUAL 0)
    target_compile_definitions(logpp PUBLIC LOGPP_CORES=${LOGPP_CORES})
endif()

include(GenerateExportHeader)
generate_export_header(logpp
    EXPORT_MACRO_NAME LOGPP_API
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/logpp/core/common.h
)

find_package(Threads REQUIRED)

target_compile_features(logpp PUBLIC cxx_std_17)
target_compile_options(logpp PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -pipe -march=native
        $<$<CONFIG:Debug>:-O0 -Wall -Wextra>>
    $<$<CXX_COMPILER_ID:MSVC>:
        $<$<CONFIG:Debug>:/Od /Wall /Zi>>
)

target_include_directories(logpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(logpp PUBLIC ${CONAN_LIBS_FMT} ${CONAN_LIBS_TOMLPLUSPLUS} Threads::Threads)

set_target_properties(logpp PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)