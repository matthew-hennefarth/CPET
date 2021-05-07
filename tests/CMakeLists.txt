add_executable(runUnitTests hello.cpp)

set_target_properties( runUnitTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/test )

# Link external libraries
target_link_libraries_system(runUnitTests PRIVATE spdlog::spdlog cxxopts Eigen3::Eigen CsLibGuarded gtest_main)

# Link Normal libraries
target_link_libraries(runUnitTests PRIVATE Threads::Threads)

include(GoogleTest)
gtest_discover_tests(runUnitTests)

add_test(
    NAME runUnitTests
    COMMAND runUnitTests
)
