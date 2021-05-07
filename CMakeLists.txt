cmake_minimum_required(VERSION 3.11)
project(CPET VERSION 0.0.0)

if( NOT CMAKE_BUILD_TYPE )
  message("Setting to CMAKE_BUILD_TYPE to Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_NO_SYSTEM_FROM_IMPORTED OFF)

configure_file(${PROJECT_SOURCE_DIR}/cmake/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

if( CMAKE_BUILD_TYPE MATCHES Debug )
  add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
  # Set the binary output
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/Debug )
else()
  add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
  # Set the binary output
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/ )
endif()

#-----------------------------------------[INCLUDE COMPILER WARNINGS]-----------------------------------------
include(cmake/compilerWarnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/StandardCmake.cmake)
#--------------------------------------------[EXTERNAL LIBRARIES]---------------------------------------------

include(cmake/linkExternal.cmake)
include(cmake/CPM.cmake)

CPMAddPackage(
  NAME eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen
  GIT_TAG origin/master
  OPTIONS "EIGEN_BUILD_DOC NO" "BUILD_TESTING NO" ""
)
CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  GIT_TAG origin/master
)
CPMAddPackage(
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.8.5
)
CPMAddPackage(
  GITHUB_REPOSITORY copperspice/cs_libguarded
  GIT_TAG origin/master
)

#----[Source]----
add_subdirectory(src)
