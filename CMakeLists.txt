cmake_minimum_required(VERSION 3.10)
project(logpp VERSION 0.1.0 LANGUAGES CXX)

# Determine if logpp is built as a subproject (using add_subdirectory)
# or if it is the master project.
if (NOT DEFINED LOGPP_MASTER_PROJECT)
  set(LOGPP_MASTER_PROJECT OFF)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(LOGPP_MASTER_PROJECT ON)
  endif()
endif()

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

option(LOGPP_INSTAL "Generate install target" ${LOGPP_MASTER_PROJECT})
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

add_custom_target(version
    COMMAND
        ${CMAKE_COMMAND} -E echo "${CMAKE_PROJECT_VERSION}"
)

get_target_property(_logpp_type logpp TYPE)
if (_logpp_type STREQUAL STATIC_LIBRARY)
    set(_type "static")
elseif (_logpp_type STREQUAL SHARED_LIBRARY)
    set(_type "shared")
else()
    message(FATAL_ERROR "Unknown library type")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(_comp "GCC")
else()
    set(_comp "${CMAKE_CXX_COMPILER_ID}")
endif()

set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /.vscode
    /.clangd
    .gitignore
    /.*build.*
    compile_commands.json
)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/package")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURENT_BINARY_DIR})
string(TOLOWER ${CMAKE_SYSTEM_NAME} _sys)
string(TOLOWER ${PROJECT_NAME} _project_lower)
string(TOLOWER ${_comp} _comp)
set(CPACK_PACKAGE_FILE_NAME "${_project_lower}-${_sys}-${_comp}-${_type}-${PROJECT_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${_project_lower}-src-${PROJECT_VERSION}")

include(CPack)
