cmake_minimum_required(VERSION 3.10)
project(logpp VERSION 0.1.0 LANGUAGES CXX)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  message(STATUS "[Deps] Using conan")
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()

  set(LOGPP_LIBS_FMT ${CONAN_LIBS_FMT})
  set(LOGPP_LIBS_TOMLPLUSPLUS ${CONAN_LIBS_TOMLPLUSPLUS})
else()
  if(LOGPP_DEPS_FMT)
    message(STATUS "[Deps] Using ${LOGPP_DEPS_FMT}")
    set(LOGPP_LIBS_FMT ${LOGPP_DEPS_FMT})
  endif()
  if(LOGPP_DEPS_TOMLPLUSPLUS)
    message(STATUS "[Deps] Using ${LOGPP_DEPS_TOMLPLUSPLUS}")
    set(LOGPP_LIBS_TOMLPLUSPLUS ${LOGPP_DEPS_TOMLPLUSPLUS})
  endif()
endif()

set(CMAKE_DEBUG_POSTFIX d)

option(LOGPP_SHARED "Build logpp as a shared library" ON)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHES "Build benches" OFF)

enable_testing()
add_subdirectory(src)
if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
if (BUILD_BENCHES)
  add_subdirectory(benches)
endif()
add_subdirectory(examples)

add_custom_target(format
    COMMAND
        ./tools/format.sh
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)
