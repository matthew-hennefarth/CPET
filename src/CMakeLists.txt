set( SOURCE_FILES main.cpp Utilities.cpp System.cpp Option.cpp Calculator.cpp)
include_directories( ${PROJECT_SOURCE_DIR}/include )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#---------------------------------------------------[Main Executable]---------------------------------------------------
add_executable(cpet ${SOURCE_FILES})
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    message("Enabling sanitizers")
    enable_sanitizers(cpet)
    message("Enabling warnings")
    enable_warnings(cpet)
endif()
# Link external libraries
target_link_libraries_system(cpet PRIVATE spdlog::spdlog cxxopts Eigen3::Eigen CsLibGuarded)

#target_link_libraries(cpet PUBLIC spdlog::spdlog)
#target_link_libraries(cpet PUBLIC cxxopts)
#target_link_libraries(cpet PRIVATE Eigen3::Eigen)
#target_link_libraries(cpet PUBLIC CsLibGuarded)

# Link Normal libraries
target_link_libraries(cpet PRIVATE Threads::Threads)
