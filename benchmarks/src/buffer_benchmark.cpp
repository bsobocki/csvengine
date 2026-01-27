#include<benchmark/benchmark.h>

#include <csvbuffer.hpp>
#include <csvreader.hpp>
#include <csvconfig.hpp>

#include <helpers.hpp>
#include <testdata.hpp>

namespace csv {

// Benchmark: Reader with custom buffer sizes (Buffer<N>)
// This exercises Buffer::compact() + refill patterns more directly.
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

        // Use your Buffer<N>(istream) constructor
        auto buffer = std::make_unique<Buffer<N>>(std::move(stream));
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
BENCHMARK(BM_Reader_BufferSized_EndToEnd<64>)->Arg(50);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<256>)->Arg(50);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<4096>)->Arg(50);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<65536>)->Arg(50);

}