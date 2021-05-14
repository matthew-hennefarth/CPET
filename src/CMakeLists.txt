set( SOURCE_FILES main.cpp Utilities.cpp System.cpp Option.cpp Calculator.cpp EFieldVolume.cpp)
include_directories( ${PROJECT_SOURCE_DIR}/include )

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)

#---------------------------------------------------[Main Executable]---------------------------------------------------
add_executable(cpet ${SOURCE_FILES})
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  target_link_libraries(cpet project_options project_warnings)
  target_compile_definitions(cpet PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
else()
  target_compile_definitions(cpet PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif()
# Link external libraries
target_link_libraries_system(cpet PUBLIC spdlog::spdlog cxxopts Eigen3::Eigen CsLibGuarded)

#target_link_libraries(cpet PUBLIC spdlog::spdlog)
#target_link_libraries(cpet PUBLIC cxxopts)
#target_link_libraries(cpet PRIVATE Eigen3::Eigen)
#target_link_libraries(cpet PUBLIC CsLibGuarded)

# Link Normal libraries
target_link_libraries(cpet PUBLIC Threads::Threads)
