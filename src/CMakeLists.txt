find_package(Filesystem REQUIRED)

set(SOURCE_FILES
  AsyncQueuePoller.cpp
  FileSink.cpp
  FileWatcher.cpp
  LogBuffer.cpp
  LogBufferView.cpp
  LoggerRegistry.cpp
  LogFmtFormatter.cpp
  PatternFormatter.cpp
  RollingFileSink.cpp
  SpinWait.cpp
  TomlConfigurator.cpp
  tz.cpp
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

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

target_include_directories(logpp
  PUBLIC
    $<BUILD_INTERFACE:${PARENT_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(logpp
  PUBLIC
    ${LOGPP_LIBS_FMT}
    ${LOGPP_LIBS_TOMLPLUSPLUS}

    std::filesystem
    Threads::Threads
)

set_target_properties(logpp
  PROPERTIES
    DEBUG_POSTFIX d
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if (LOGPP_INSTAL)
  install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/include/logpp"
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
  )

  install(
    TARGETS logpp
    EXPORT logppTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
  )

  install(
    EXPORT logppTargets
    FILE logppTargets.cmake
    NAMESPACE logpp::
    DESTINATION lib
  )

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    logppConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
  )

  install(
    FILES
      "${CMAKE_SOURCE_DIR}/cmake/logpp-config.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/logppConfigVersion.cmake"
    DESTINATION
      lib
  )
endif()
