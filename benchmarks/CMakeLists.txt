add_executable(run_benchmarks
  src/helpers.cpp
  src/parser_benchmark.cpp
  src/reader_benchmark.cpp
  src/streambuffer_benchmark.cpp
  src/buffers_comparison_benchmark.cpp
  src/record_vs_recordview_benchmark.cpp
  src/external_comparison_benchmark.cpp
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

if(CSVENGINE_BUILD_EXTERNAL_BENCHMARKS)
    target_link_libraries(run_benchmarks PRIVATE fast_cpp_csv_parser)
    target_compile_definitions(run_benchmarks PRIVATE CSVENGINE_HAVE_FAST_CPP_CSV_PARSER=1)
endif()