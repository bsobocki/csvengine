# Minimum CMake version required
cmake_minimum_required(VERSION 3.20)

# Project name and language
project(csvengine VERSION 1.0 LANGUAGES CXX)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "
    In-source builds are not allowed.
    Please create a separate build directory:
        mkdir build
        cd build
        cmake ..
    ")
endif()

# Standard C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# Enable CTest
enable_testing()

include(FetchContent)

# Fetch Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

# Fetch Google Benchmark - DISABLE ITS TESTS FIRST
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark gtest tests" FORCE)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.5
)
FetchContent_MakeAvailable(benchmark)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include Subdirectories
add_subdirectory(engine)
add_subdirectory(demo)
add_subdirectory(tests)
add_subdirectory(benchmarks)
