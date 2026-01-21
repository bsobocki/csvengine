add_executable(run_benchmarks
  src/parser_bench.cpp
  src/reader_bench.cpp
)

target_link_libraries(run_benchmarks
  PRIVATE
    benchmark::benchmark_main
    csvengine
)
