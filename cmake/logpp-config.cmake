include(CMakeFindDependencyMacro)

find_dependency(fmt REQUIRED)
find_dependency(tomlplusplus REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/logppTargets.cmake")
