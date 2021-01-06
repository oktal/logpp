cmake_minimum_required(VERSION 3.10)
project(logpp VERSION 0.1.0 LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

option(LOGPP_SHARED "Build logpp as a shared library" ON)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)