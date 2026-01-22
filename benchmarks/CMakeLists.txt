add_executable(run_benchmarks
  src/parser_bench.cpp
  src/reader_bench.cpp
)

target_link_libraries(run_benchmarks
  PRIVATE
    benchmark::benchmark_main
    csvengine
)

target_include_directories(run_benchmarks
  PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_data
)