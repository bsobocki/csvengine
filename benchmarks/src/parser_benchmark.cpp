#include <benchmark/benchmark.h>

#include <csvreader.hpp>
#include <csvbuffer.hpp>
#include <csvconfig.hpp>

#include <testdata.hpp>
#include <helpers.hpp>

namespace csv {

static void BM_SimpleData_ParserComparison_TestBody(benchmark::State& state, Config& cfg, const std::string& data = simple_csv_data) {
    const int repeats = static_cast<int>(state.range(0));

    const std::string csv_text = repeat_csv(data, repeats);

    // if i don't specify 'Iterations' in BENCHMARK(...)->Arg(x)->Iterations(y)
    // google benchmark will decide how many iterations it will do
    for (auto _ : state) {
        auto stream = std::make_unique<std::istringstream>(csv_text);
        Reader reader(std::move(stream), cfg);

        int64_t rows = 0;
        while (reader.next()) {
            rows++;
            auto record = reader.current_record();
            benchmark::DoNotOptimize(record);
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

}