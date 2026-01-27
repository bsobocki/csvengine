#include <benchmark/benchmark.h>

#include <csvreader.hpp>
#include <csvbuffer.hpp>
#include <csvconfig.hpp>

#include <testdata.hpp>
#include <helpers.hpp>
#include <sstream>
#include <string>

namespace csv {

constexpr int64_t small_data  = 50;
constexpr int64_t medium_data = 1000;
constexpr int64_t big_data    = 10000;

// Benchmark: iterate through entire CSV using Reader(stream)
// Measures end-to-end: buffering + parsing + record construction
static void BM_Reader_Stream_EndToEnd(benchmark::State& state) {
    const int repeats = static_cast<int>(state.range(0));

    // Prepare data outside the measured loop
    const std::string csv_text = repeat_csv(simple_csv_data, repeats);

    // Config: typical default strict quoting, header present in your sample
    Config cfg{};
    cfg.has_header = true;
    cfg.parse_mode = Config::ParseMode::strict;
    cfg.has_quoting = true;
    cfg.line_ending = Config::LineEnding::lf;

    std::size_t total_rows = 0;

    for (auto _ : state) {
        // Setup inside each iteration so every run parses from scratch
        auto stream = std::make_unique<std::istringstream>(csv_text);
        Reader reader(std::move(stream), cfg);

        while (reader.next()) {
            const auto& rec = reader.current_record();
            total_rows++;
            benchmark::DoNotOptimize(rec);
        }

        benchmark::DoNotOptimize(total_rows);
    }

    // Report throughput
    state.SetItemsProcessed(static_cast<int64_t>(total_rows));
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * csv_text.size());
}
BENCHMARK(BM_Reader_Stream_EndToEnd)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data);

// Benchmark: quoted-heavy dataset (uses your quoted_csv_data)
static void BM_Reader_QuotedData_EndToEnd(benchmark::State& state) {
    const int repeats = static_cast<int>(state.range(0));
    const std::string csv_text = repeat_csv(quoted_csv_data, repeats);

    Config cfg{};
    cfg.has_header = true;
    cfg.parse_mode = Config::ParseMode::strict;
    cfg.has_quoting = true;
    cfg.line_ending = Config::LineEnding::lf;

    std::size_t total_rows = 0;

    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);
        Reader reader(std::move(stream), cfg);

        while (reader.next()) {
            total_rows++;
            benchmark::DoNotOptimize(reader.current_record());
        }

        benchmark::DoNotOptimize(total_rows);
    }

    state.SetItemsProcessed(static_cast<int64_t>(total_rows));
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * csv_text.size());
}
BENCHMARK(BM_Reader_QuotedData_EndToEnd)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data);

}