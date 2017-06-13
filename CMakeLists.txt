# CMake version
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Dependencies
find_package(Numa)

add_subdirectory(src)
add_subdirectory(third_party)
