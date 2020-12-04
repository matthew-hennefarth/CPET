set( SOURCE_FILES main.cpp Utilities.cpp System.cpp Option.cpp Calculator.cpp)
include_directories( ${PROJECT_SOURCE_DIR}/include )

# Include any 3rd Party Header Files
include_directories(SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/libs/include)
 
# Include spdlog
find_library(SPDLOG spdlog "${CMAKE_SOURCE_DIR}/libs/lib")

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
target_link_libraries(cpet PRIVATE ${SPDLOG})
target_link_libraries(cpet PRIVATE Threads::Threads)
