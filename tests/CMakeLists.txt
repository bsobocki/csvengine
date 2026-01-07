# Create the executable for our tests
add_executable(run_tests 
  tests.cpp
  csvreader_test.cpp
  csvbuffer_test.cpp
  csvparser_tests/csvparser_quoting_lenient_test.cpp
  csvparser_tests/csvparser_quoting_strict_test.cpp
  csvparser_tests/csvparser_simple_test.cpp
  csvrecord_test.cpp
)

# Link our test executable against:
# 1. gtest_main: Google Test's main function and library
# 2. csvengine: Our library that we want to test
target_link_libraries(run_tests
  PRIVATE
    gtest_main
    gmock
    csvengine
)

target_include_directories(run_tests
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# Copy test_data directory to the build directory
add_custom_command(TARGET run_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data
    ${CMAKE_CURRENT_BINARY_DIR}/test_data
)

# Find all tests defined with the TEST() macro and add them to CTest
include(GoogleTest)
gtest_discover_tests(run_tests)
