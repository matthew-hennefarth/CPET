include_directories( ${PROJECT_SOURCE_DIR}/include )

add_executable(runUnitTests test_utilities.cpp test_volume.cpp test_pointcharges.cpp ../src/Utilities.cpp)
set_target_properties( runUnitTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/test )
target_compile_definitions(runUnitTests PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)

# Link external libraries
target_link_libraries_system(runUnitTests spdlog::spdlog cxxopts Eigen3::Eigen CsLibGuarded gtest_main)

# Link Normal libraries
target_link_libraries(runUnitTests Threads::Threads)

include(GoogleTest)
gtest_discover_tests(runUnitTests)

add_test(
    NAME runUnitTests
    COMMAND runUnitTests
)
