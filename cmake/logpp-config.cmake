include(CMakeFindDependencyMacro)

find_dependency(fmt 12.2.0 CONFIG REQUIRED)
find_dependency(tomlplusplus REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/logppTargets.cmake")
