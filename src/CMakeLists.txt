set( SOURCE_FILES main.cpp Utilities.cpp System.cpp Option.cpp Calculator.cpp EFieldVolume.cpp
    Volume.cpp FieldLocations.cpp TopologyRegion.cpp Histogram2D.cpp)
include_directories( ${PROJECT_SOURCE_DIR}/include )

#---------------------------------------------------[Main Executable]---------------------------------------------------
add_executable(cpet ${SOURCE_FILES})
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  target_link_libraries(cpet PRIVATE project_options project_warnings)
  target_compile_definitions(cpet PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
else()
  target_compile_definitions(cpet PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif()

if(ENABLE_STATIC_LINKING)
  if(APPLE)
    find_package(JPEG REQUIRED)
    find_package(TIFF REQUIRED)
    find_package(PNG REQUIRED)
    target_link_libraries(cpet PUBLIC JPEG::JPEG TIFF::TIFF PNG::PNG)
  else()
    target_link_libraries(cpet PUBLIC -static)
  endif()
endif()

# Link 3rd party, external libraries these are all static
target_link_libraries_system(cpet PUBLIC spdlog::spdlog cxxopts Eigen3::Eigen CsLibGuarded matplot)
