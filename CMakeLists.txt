cmake_minimum_required(VERSION 3.10)
project(logpp VERSION 0.1.0 LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

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