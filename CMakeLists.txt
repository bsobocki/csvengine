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
set(CMAKE_CXX_EXTENSIONS OFF) # we stick to standard C++

add_compile_options(-Wall -Wextra -Wpedantic)

# Enable CTest, the testing utility from CMake
enable_testing()

# Fetch Google Test
# Include the FetchContent module to download dependencies
include(FetchContent)

# Declare the GTest dependency from its GitHub repository
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0 # Use a specific, stable version
)

# Make GTest available to our project. CMake will download and configure it.
FetchContent_MakeAvailable(googletest)

# Generate compile_commands.json to make VS Code IntelliSense happy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include Subdirectories
add_subdirectory(engine)
add_subdirectory(demo)
add_subdirectory(tests)
