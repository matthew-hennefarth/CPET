cmake_minimum_required(VERSION 3.11)
project(CPET)

if( NOT CMAKE_BUILD_TYPE )
    message("Setting to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

if( CMAKE_BUILD_TYPE MATCHES Debug )
  add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
  # Set the binary output
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/Debug )
else()
  add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
  # Set the binary output
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/ )
endif()

#----------------------------------------------[INCLUDE COMPILER WARNINGS]----------------------------------------------
include(cmake/compilerWarnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/StandardCmake.cmake)

add_subdirectory(src)
