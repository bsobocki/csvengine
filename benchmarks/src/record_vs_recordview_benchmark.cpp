#include <benchmark/benchmark.h>

#include <csvreader/csvreader.hpp>
#include <csvconfig.hpp>

#include <testdata.hpp>
#include <helpers.hpp>

namespace csv {

constexpr int64_t iterations  = 50;
constexpr int64_t small_data  = 100;
constexpr int64_t medium_data = 1000;
constexpr int64_t big_data    = 10000;

template<class ReaderType>
static void BM_RecordComparison_TestBody(benchmark::State& state, Config& cfg, const std::string& data) {
    const int repeats = static_cast<int>(state.range(0));
    const std::string csv_text = repeat_csv(data, repeats);

    int64_t total_rows = 0;

    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);
        ReaderType reader(std::move(stream), cfg);

        int64_t rows = 0;
        while (reader.next()) {
            rows++;
            auto record = reader.current_record();
            benchmark::DoNotOptimize(record);
        }
        benchmark::DoNotOptimize(rows);
        total_rows += rows;
    }

    state.SetItemsProcessed(total_rows);
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * csv_text.size());
}

static void BM_RecordComparison_Record_SimpleParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = false,
        .line_ending = Config::LineEnding::lf,
    };
    BM_RecordComparison_TestBody<Reader>(state, cfg, simple_csv_data);
}

static void BM_RecordComparison_RecordView_SimpleParser(benchmark::State& state) {
    Config cfg{
        .has_header = true,
        .has_quoting = false,
        .line_ending = Config::LineEnding::lf,
    };
    BM_RecordComparison_TestBody<ViewReader>(state, cfg, simple_csv_data);
}

BENCHMARK(BM_RecordComparison_Record_SimpleParser)->Arg(small_data)->Iterations(iterations);
BENCHMARK(BM_RecordComparison_RecordView_SimpleParser)->Arg(small_data)->Iterations(iterations);

BENCHMARK(BM_RecordComparison_Record_SimpleParser)->Arg(medium_data)->Iterations(iterations);
BENCHMARK(BM_RecordComparison_RecordView_SimpleParser)->Arg(medium_data)->Iterations(iterations);

BENCHMARK(BM_RecordComparison_Record_SimpleParser)->Arg(big_data)->Iterations(iterations);
BENCHMARK(BM_RecordComparison_RecordView_SimpleParser)->Arg(big_data)->Iterations(iterations);

}