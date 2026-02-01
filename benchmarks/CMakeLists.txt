add_executable(run_benchmarks
  src/helpers.cpp
  src/parser_benchmark.cpp
  src/reader_benchmark.cpp
  src/streambuffer_benchmark.cpp
)

target_link_libraries(run_benchmarks
  PRIVATE
    benchmark::benchmark_main
    csvengine
)

target_include_directories(run_benchmarks
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/tests/test_data
)