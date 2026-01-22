#include <benchmark/benchmark.h>

#include <csvreader.hpp>
#include <csvbuffer.hpp>
#include <csvconfig.hpp>

#include <testdata.hpp>   // reusing your test strings
#include <sstream>
#include <string>

using namespace csv;

// Utility: repeat an input CSV N times.
// (Ensures bigger data sizes so benchmarks are meaningful.)
static std::string repeat_csv(std::string_view one_file, int repeats) {
    std::string out;
    out.reserve(one_file.size() * repeats);
    for (int i = 0; i < repeats; ++i) {
        out.append(one_file);
        // ensure last record ends with newline to avoid “EOF flush” differences
        if (!out.empty() && out.back() != '\n') out.push_back('\n');
    }
    return out;
}

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

    for (auto _ : state) {
        // Setup inside each iteration so every run parses from scratch
        auto stream = std::make_unique<std::istringstream>(csv_text);
        Reader reader(std::move(stream), cfg);

        std::size_t rows = 0;
        std::size_t bytes = 0;

        while (reader.next()) {
            const auto& rec = reader.current_record();
            rows++;
            for (const auto& f : rec.fields()) bytes += f.size();
            benchmark::DoNotOptimize(rec);
        }

        benchmark::DoNotOptimize(rows);
        benchmark::DoNotOptimize(bytes);

        // Report throughput
        state.SetItemsProcessed(static_cast<int64_t>(rows));
        state.SetBytesProcessed(static_cast<int64_t>(csv_text.size()));
    }
}
BENCHMARK(BM_Reader_Stream_EndToEnd)
    ->Arg(1)->Arg(10)->Arg(100); // repeats


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

    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);

        // Use your Buffer<N>(istream) constructor
        auto buffer = std::make_unique<Buffer<N>>(std::move(stream));
        Reader reader(std::move(buffer), cfg);

        std::size_t rows = 0;
        while (reader.next()) {
            rows++;
            benchmark::DoNotOptimize(reader.current_record());
        }

        benchmark::DoNotOptimize(rows);
        state.SetItemsProcessed(static_cast<int64_t>(rows));
        state.SetBytesProcessed(static_cast<int64_t>(csv_text.size()));
    }
}

// Register a few sizes (tiny -> default-ish)
BENCHMARK(BM_Reader_BufferSized_EndToEnd<64>)->Arg(50);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<256>)->Arg(50);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<4096>)->Arg(50);
BENCHMARK(BM_Reader_BufferSized_EndToEnd<65536>)->Arg(50);


// Benchmark: quoted-heavy dataset (uses your quoted_csv_data)
static void BM_Reader_QuotedData_EndToEnd(benchmark::State& state) {
    const int repeats = static_cast<int>(state.range(0));
    const std::string csv_text = repeat_csv(quoted_csv_data, repeats);

    Config cfg{};
    cfg.has_header = true;
    cfg.parse_mode = Config::ParseMode::strict;
    cfg.has_quoting = true;
    cfg.line_ending = Config::LineEnding::lf;

    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);
        Reader reader(std::move(stream), cfg);

        std::size_t rows = 0;
        while (reader.next()) {
            rows++;
            benchmark::DoNotOptimize(reader.current_record());
        }

        benchmark::DoNotOptimize(rows);
        state.SetItemsProcessed(static_cast<int64_t>(rows));
        state.SetBytesProcessed(static_cast<int64_t>(csv_text.size()));
    }
}
BENCHMARK(BM_Reader_QuotedData_EndToEnd)->Arg(1)->Arg(50);
