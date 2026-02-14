#include <benchmark/benchmark.h>

#include <csvreader/csvreader.hpp>
#include <csvconfig.hpp>

#include <testdata.hpp>
#include <helpers.hpp>
#include <sstream>
#include <string>

namespace csv {

constexpr int64_t small_data  = 100;
constexpr int64_t medium_data = 1000;
constexpr int64_t big_data    = 10000;
constexpr int64_t huge_data   = 1000000;

class BuffersComparisonFixture : public benchmark::Fixture {
public:
    BuffersComparisonFixture(const std::string& csv_data): csv_data_(csv_data) {}

    std::string filename_;
    std::string csv_data_;
    std::string csv_file_content_;

    void SetUp(const ::benchmark::State& state) override {
        int repeats = static_cast<int>(state.range(0));
        filename_ = "buffers_comparison_benchmark_" + std::to_string(repeats) + ".tmp";

        csv_file_content_ = repeat_csv(csv_data_, repeats);

        std::ofstream out(filename_, std::ios::binary);
        out << csv_file_content_;
        out.close();
    }

    void TearDown(const ::benchmark::State& state) override {
        std::remove(filename_.c_str());
    }

    void benchmark_body(benchmark::State& state, Config cfg) {
        cfg.streaming = true;
        cfg.has_header = true;
        cfg.has_quoting = true;
        cfg.parse_mode = Config::ParseMode::strict;
        cfg.line_ending = Config::LineEnding::lf;

        std::size_t total_rows = 0;

        for (auto _ : state) {
            Reader reader(filename_, cfg);

            while (reader.next()) {
                const auto& rec = reader.current_record();
                total_rows++;
                benchmark::DoNotOptimize(rec);
            }

            benchmark::DoNotOptimize(total_rows);
        }

        state.SetItemsProcessed(static_cast<int64_t>(total_rows));
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * csv_file_content_.size());
    }
};

class BuffersComparisonSimpleDataFixture : public BuffersComparisonFixture {
public:
    BuffersComparisonSimpleDataFixture() : BuffersComparisonFixture(simple_csv_data){}
};

class BuffersComparisonQuotedDataFixture : public BuffersComparisonFixture {
public:
    BuffersComparisonQuotedDataFixture() : BuffersComparisonFixture(quoted_csv_data){}
};

BENCHMARK_DEFINE_F(BuffersComparisonSimpleDataFixture, StreamBuffer_Simple)(benchmark::State& state) {
    Config cfg {
        .mapped_buffer = false,
    };

    benchmark_body(state, cfg);
}

BENCHMARK_DEFINE_F(BuffersComparisonSimpleDataFixture, MappedBuffer_Simple)(benchmark::State& state) {
    Config cfg {
        .mapped_buffer = true,
    };

    benchmark_body(state, cfg);
}

BENCHMARK_DEFINE_F(BuffersComparisonQuotedDataFixture, StreamBuffer_Quoted)(benchmark::State& state) {
    Config cfg {
        .mapped_buffer = false,
    };

    benchmark_body(state, cfg);
}
BENCHMARK_DEFINE_F(BuffersComparisonQuotedDataFixture, MappedBuffer_Quoted)(benchmark::State& state) {
    Config cfg {
        .mapped_buffer = true,
    };

    benchmark_body(state, cfg);   
}

BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, StreamBuffer_Simple)->Arg(small_data);
BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, MappedBuffer_Simple)->Arg(small_data);
BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, StreamBuffer_Simple)->Arg(medium_data);
BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, MappedBuffer_Simple)->Arg(medium_data);
BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, StreamBuffer_Simple)->Arg(big_data);
BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, MappedBuffer_Simple)->Arg(big_data);
BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, StreamBuffer_Simple)->Arg(huge_data);
BENCHMARK_REGISTER_F(BuffersComparisonSimpleDataFixture, MappedBuffer_Simple)->Arg(huge_data);

BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, StreamBuffer_Quoted)->Arg(small_data);
BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, MappedBuffer_Quoted)->Arg(small_data);
BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, StreamBuffer_Quoted)->Arg(medium_data);
BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, MappedBuffer_Quoted)->Arg(medium_data);
BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, StreamBuffer_Quoted)->Arg(big_data);
BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, MappedBuffer_Quoted)->Arg(big_data);
BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, StreamBuffer_Quoted)->Arg(huge_data);
BENCHMARK_REGISTER_F(BuffersComparisonQuotedDataFixture, MappedBuffer_Quoted)->Arg(huge_data);
}