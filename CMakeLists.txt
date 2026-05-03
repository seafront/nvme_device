cmake_minimum_required(VERSION 3.16)
project(nvme_device VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# Library
add_library(nvme_device STATIC
    src/nvme_device.cpp
)
target_include_directories(nvme_device PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Example binary
add_executable(basic_io examples/basic_io.cpp)
target_link_libraries(basic_io PRIVATE nvme_device)

# Tests
find_package(GTest REQUIRED)
enable_testing()

file(GLOB TEST_SOURCES tests/*.cpp)
if(TEST_SOURCES)
    add_executable(nvme_tests ${TEST_SOURCES})
    target_link_libraries(nvme_tests PRIVATE nvme_device GTest::gtest_main GTest::gmock)
    include(GoogleTest)
    gtest_discover_tests(nvme_tests)
endif()
