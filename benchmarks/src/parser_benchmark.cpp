#include <benchmark/benchmark.h>

#include <csvreader.hpp>
#include <csvbuffer.hpp>
#include <csvconfig.hpp>

#include <testdata.hpp>
#include <helpers.hpp>

namespace csv {

constexpr int64_t iterations  = 50;
constexpr int64_t small_data  = 100;
constexpr int64_t medium_data = 1000;
constexpr int64_t big_data    = 10000;

static void BM_ParserComparison_TestBody(benchmark::State& state, Config& cfg, const std::string& data) {
    const int repeats = static_cast<int>(state.range(0));
    const std::string csv_text = repeat_csv(data, repeats);

    int64_t total_rows = 0;

    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);
        Reader reader(std::move(stream), cfg);

        int64_t rows = 0;
        while (reader.next()) {
            rows++;
            benchmark::DoNotOptimize(reader.current_record());
        }
        benchmark::DoNotOptimize(rows);
        total_rows += rows;
    }

    state.SetItemsProcessed(total_rows);
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * csv_text.size());
}


// === Simple Data ===

static void BM_SimpleData_ParserComparison_SimpleParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = false,
        .line_ending = Config::LineEnding::lf,
    };
    BM_ParserComparison_TestBody(state, cfg, simple_csv_data);
}
static void BM_SimpleData_ParserComparison_StrictParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = true,
        .parse_mode = Config::ParseMode::strict,
        .line_ending = Config::LineEnding::lf,
    };
    BM_ParserComparison_TestBody(state, cfg, simple_csv_data);
}
static void BM_SimpleData_ParserComparison_LenientParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = true,
        .parse_mode = Config::ParseMode::lenient,
        .line_ending = Config::LineEnding::lf,
    };
    BM_ParserComparison_TestBody(state, cfg, simple_csv_data);
}
BENCHMARK(BM_SimpleData_ParserComparison_SimpleParser)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_SimpleData_ParserComparison_StrictParser)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_SimpleData_ParserComparison_LenientParser)->Arg(small_data)->Iterations(iterations);

BENCHMARK(BM_SimpleData_ParserComparison_SimpleParser)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_SimpleData_ParserComparison_StrictParser)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_SimpleData_ParserComparison_LenientParser)->Arg(medium_data)->Iterations(iterations);

BENCHMARK(BM_SimpleData_ParserComparison_SimpleParser)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_SimpleData_ParserComparison_StrictParser)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_SimpleData_ParserComparison_LenientParser)->Arg(big_data)->Iterations(iterations);


// === Quoted Data ===

static void BM_QuotedData_ParserComparison_StrictParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = true,
        .parse_mode = Config::ParseMode::strict,
        .line_ending = Config::LineEnding::lf,
    };
    BM_ParserComparison_TestBody(state, cfg, quoted_csv_data);
}
static void BM_QuotedData_ParserComparison_LenientParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = true,
        .parse_mode = Config::ParseMode::lenient,
        .line_ending = Config::LineEnding::lf,
    };
    BM_ParserComparison_TestBody(state, cfg, quoted_csv_data);
}
BENCHMARK(BM_QuotedData_ParserComparison_StrictParser)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_QuotedData_ParserComparison_LenientParser)->Arg(small_data)->Iterations(iterations);

BENCHMARK(BM_QuotedData_ParserComparison_StrictParser)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_QuotedData_ParserComparison_LenientParser)->Arg(medium_data)->Iterations(iterations);

BENCHMARK(BM_QuotedData_ParserComparison_StrictParser)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_QuotedData_ParserComparison_LenientParser)->Arg(big_data)->Iterations(iterations);

}