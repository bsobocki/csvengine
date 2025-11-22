# Create the executable for our tests
add_executable(run_tests tests.cpp)

# Link our test executable against:
# 1. gtest_main: Google Test's main function and library
# 2. csvengine: Our library that we want to test
target_link_libraries(run_tests
  PRIVATE
    gtest_main
    csvengine
)

# Find all tests defined with the TEST() macro and add them to CTest
include(GoogleTest)
gtest_discover_tests(run_tests)
