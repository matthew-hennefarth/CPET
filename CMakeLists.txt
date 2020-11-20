cmake_minimum_required(VERSION 3.17)
project(CPET)

if( NOT CMAKE_BUILD_TYPE )
    message("Setting to Release")
    set( CMAKE_BUILD_TYPE Release ... FORCE )
endif()


set(CMAKE_CXX_STANDARD 20)

# Set the binary output
if( CMAKE_BUILD_TYPE MATCHES Debug )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )
    add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
elseif( CMAKE_BUILD_TYPE MATCHES Release )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/release )
endif()

#----------------------------------------------[INCLUDE COMPILER WARNINGS]----------------------------------------------
include(cmake/compilerWarnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/StandardCmake.cmake)

add_subdirectory(src)