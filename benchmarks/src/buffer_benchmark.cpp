#include<benchmark/benchmark.h>

#include <csvbuffer/csvstreambuffer.hpp>
#include <csvreader.hpp>
#include <csvconfig.hpp>

#include <helpers.hpp>
#include <testdata.hpp>

namespace csv {

constexpr int64_t iterations  = 50;
constexpr int64_t small_data  = 100;
constexpr int64_t medium_data = 1000;
constexpr int64_t big_data    = 10000;

// Benchmark: Reader with custom buffer sizes (StreamBuffer<N>)
// This exercises StreamBuffer::compact() + refill patterns more directly.
template <size_t N>
static void BM_Reader_BufferSized_EndToEnd(benchmark::State& state) {
    const int repeats = static_cast<int>(state.range(0));
    const std::string csv_text = repeat_csv(simple_csv_data, repeats);

    Config cfg{};
    cfg.has_header = true;
    cfg.parse_mode = Config::ParseMode::strict;
    cfg.has_quoting = true;
    cfg.line_ending = Config::LineEnding::lf;

    std::size_t total_rows = 0;

    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);

        // Use your StreamBuffer<N>(istream) constructor
        auto buffer = std::make_unique<StreamBuffer<N>>(std::move(stream));
        Reader reader(std::move(buffer), cfg);

        while (reader.next()) {
            total_rows++;
            benchmark::DoNotOptimize(reader.current_record());
        }

        benchmark::DoNotOptimize(total_rows);
    }
    state.SetItemsProcessed(static_cast<int64_t>(total_rows));
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * csv_text.size());
}

// Register a few sizes (tiny -> default-ish)
BENCHMARK(BM_Reader_BufferSized_EndToEnd<64>)->Arg(small_data)->Arg(medium_data)->Arg(big_data);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<256>)->Arg(small_data)->Arg(medium_data)->Arg(big_data);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<1024>)->Arg(small_data)->Arg(medium_data)->Arg(big_data);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<2048>)->Arg(small_data)->Arg(medium_data)->Arg(big_data);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<4096>)->Arg(small_data)->Arg(medium_data)->Arg(big_data);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<65536>)->Arg(small_data)->Arg(medium_data)->Arg(big_data);

BENCHMARK(BM_Reader_BufferSized_EndToEnd<64>)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<256>)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<1024>)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<2048>)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<4096>)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<65536>)->Arg(small_data)->Iterations(iterations);

BENCHMARK(BM_Reader_BufferSized_EndToEnd<64>)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<256>)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<1024>)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<2048>)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<4096>)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<65536>)->Arg(medium_data)->Iterations(iterations);

BENCHMARK(BM_Reader_BufferSized_EndToEnd<64>)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<256>)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<1024>)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<2048>)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<4096>)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<65536>)->Arg(big_data)->Iterations(iterations);
}