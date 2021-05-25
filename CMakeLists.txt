cmake_minimum_required(VERSION 3.14)
project(CPET VERSION 0.1.0)
include(cmake/PreventInSourceBuilds.cmake)
include(cmake/StandardCmake.cmake)

if( NOT CMAKE_BUILD_TYPE )
  message("Setting to CMAKE_BUILD_TYPE to Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

# enable cache system
include(cmake/Cache.cmake)

configure_file(${PROJECT_SOURCE_DIR}/cmake/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

if( CMAKE_BUILD_TYPE MATCHES Debug )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/Debug )
else()
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/ )
endif()

#-----------------------------------------[INCLUDE COMPILER WARNINGS]-----------------------------------------
include(cmake/compilerWarnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/StaticAnalyzers.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)
enable_sanitizers(project_options)

# target_link_library(target project_warnings project_options)
# to enable these warnings/options

#--------------------------------------------[EXTERNAL LIBRARIES]---------------------------------------------

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(cmake/LinkExternalLibraries.cmake)
option(CPM_USE_LOCAL_PACKAGES "Try `find_package` before downloading dependencies" ON)
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME matplotplusplus
    GITHUB_REPOSITORY alandefreitas/matplotplusplus
    GIT_TAG origin/master
    OPTIONS "BUILD_WITH_PEDANTIC_WARNINGS NO" 
            "BUILD_WITH_SANITIZERS NO"
            "BUILD_HIGH_RESOLUTION_WORLD_MAP NO"
)

CPMAddPackage(
  NAME eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG origin/master
  OPTIONS "EIGEN_BUILD_DOC NO" "BUILD_TESTING NO" 
)
CPMAddPackage("gh:jarro2783/cxxopts#master")
CPMAddPackage("gh:gabime/spdlog@1.8.5")
CPMAddPackage("gh:copperspice/cs_libguarded#master")

#----- TESTING

option(ENABLE_TESTING "Enable creation of CPET tests." OFF)
if(ENABLE_TESTING)
  include(CTest)
  message("Building tests.")
  CPMAddPackage("gh:google/googletest#master")
  add_subdirectory(tests)
endif()

#----[Source]----
add_subdirectory(src)
