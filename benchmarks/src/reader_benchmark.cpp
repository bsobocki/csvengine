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
    
    if (one_file.empty()) return "";
    
    for (int i = 0; i < repeats; ++i) {
        out.append(one_file);
        // ensure last record ends with newline to avoid “EOF flush” differences
        if (one_file.back() != '\n') out.push_back('\n');
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


static void BM_SimpleData_ParserComparison_TestBody(benchmark::State& state, Config& cfg) {
    const int repeats = static_cast<int>(state.range(0));

    const std::string csv_text = repeat_csv(simple_csv_data, repeats);

    // if i don't specify 'Iterations' in BENCHMARK(...)->Arg(x)->Iterations(y)
    // google benchmark will decide how many iterations it will be
    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);
        Reader reader(std::move(stream), cfg);

        int64_t rows = 0;
        while (reader.next()) {
            rows++;
            benchmark::DoNotOptimize(reader.current_record());
        }

        benchmark::DoNotOptimize(rows);
        state.SetItemsProcessed(rows);
        state.SetBytesProcessed(csv_text.size());
    }
}
static void BM_SimpleData_ParserComparison_SimpleParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = false,
        .line_ending = Config::LineEnding::lf,
    };
    BM_SimpleData_ParserComparison_TestBody(state, cfg);
}
static void BM_SimpleData_ParserComparison_StrictParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = true,
        .parse_mode = Config::ParseMode::strict,
        .line_ending = Config::LineEnding::lf,
    };
    BM_SimpleData_ParserComparison_TestBody(state, cfg);
}
static void BM_SimpleData_ParserComparison_LenientParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = true,
        .parse_mode = Config::ParseMode::lenient,
        .line_ending = Config::LineEnding::lf,
    };
    BM_SimpleData_ParserComparison_TestBody(state, cfg);
}
BENCHMARK(BM_SimpleData_ParserComparison_SimpleParser)->Arg(100)->Iterations(100);
BENCHMARK(BM_SimpleData_ParserComparison_StrictParser)->Arg(100)->Iterations(100);
BENCHMARK(BM_SimpleData_ParserComparison_LenientParser)->Arg(100)->Iterations(100);

BENCHMARK(BM_SimpleData_ParserComparison_SimpleParser)->Arg(1000)->Iterations(100);
BENCHMARK(BM_SimpleData_ParserComparison_StrictParser)->Arg(1000)->Iterations(100);
BENCHMARK(BM_SimpleData_ParserComparison_LenientParser)->Arg(1000)->Iterations(100);

BENCHMARK(BM_SimpleData_ParserComparison_SimpleParser)->Arg(5000)->Iterations(100);
BENCHMARK(BM_SimpleData_ParserComparison_StrictParser)->Arg(5000)->Iterations(100);
BENCHMARK(BM_SimpleData_ParserComparison_LenientParser)->Arg(5000)->Iterations(100);