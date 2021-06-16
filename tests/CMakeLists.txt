include_directories( ${PROJECT_SOURCE_DIR}/include )

add_executable(runUnitTests test_utilities.cpp test_volume.cpp test_pointcharges.cpp test_option.cpp test_system.cpp 
  ../src/Utilities.cpp ../src/Option.cpp ../src/System.cpp ../src/EFieldVolume.cpp ../src/Volume.cpp ../src/FieldLocations.cpp ../src/TopologyRegion.cpp)
set_target_properties( runUnitTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Testing )
target_compile_definitions(runUnitTests PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)

# Link external libraries
target_link_libraries_system(runUnitTests PRIVATE spdlog::spdlog cxxopts Eigen3::Eigen CsLibGuarded gtest_main matplot)

include(GoogleTest)
gtest_discover_tests(runUnitTests 
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

