// ============================================================================
// File: external_comparison_benchmark.cpp
// Description: Benchmarks comparing csvengine against fast-cpp-csv-parser.
// Note: The boilerplate for this benchmark file was generated with the 
//       assistance of an AI (LLM), and subsequently reviewed and adjusted.
// ============================================================================

#include <benchmark/benchmark.h>

#include <csvreader/csvreader.hpp>
#include <csvbuffer/csvmappedbuffer.hpp>
#include <csvconfig.hpp>

#include <testdata.hpp>
#include <helpers.hpp>

#include <fstream>
#include <cstdio>
#include <string>
#include <memory>

#if defined(CSVENGINE_HAVE_FAST_CPP_CSV_PARSER)
  #include <csv.h>  // fast-cpp-csv-parser (Ben Strasser)
#endif

#if defined(CSVENGINE_HAVE_RAPIDCSV)
  #include <rapidcsv.h>
#endif

namespace csv {
namespace external_bm {

constexpr int64_t small_data  = 100;
constexpr int64_t medium_data = 1000;
constexpr int64_t big_data    = 10000;
constexpr int64_t huge_data   = 1000000;

// =============================================================
// Fixture: prepare a real on-disk file (so libs that need a path
// can mmap/open it). Used by ALL external comparison benchmarks.
// =============================================================
class ExternalComparisonFixture : public benchmark::Fixture {
public:
    explicit ExternalComparisonFixture(const std::string& csv_data)
        : csv_data_(csv_data) {}

    std::string filename_;
    std::string csv_data_;
    std::string csv_file_content_;

    void SetUp(const ::benchmark::State& state) override {
        int repeats = static_cast<int>(state.range(0));
        filename_ = "external_comparison_benchmark_" + std::to_string(repeats) + ".tmp";

        csv_file_content_ = repeat_csv(csv_data_, repeats);

        std::ofstream out(filename_, std::ios::binary);
        out << csv_file_content_;
        out.close();
    }

    void TearDown(const ::benchmark::State& /*state*/) override {
        std::remove(filename_.c_str());
    }

    void report(benchmark::State& state, std::size_t total_rows) {
        state.SetItemsProcessed(static_cast<int64_t>(total_rows));
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                                csv_file_content_.size());
    }
};

class SimpleDataFixture : public ExternalComparisonFixture {
public:
    SimpleDataFixture() : ExternalComparisonFixture(simple_csv_data) {}
};

class QuotedDataFixture : public ExternalComparisonFixture {
public:
    QuotedDataFixture() : ExternalComparisonFixture(quoted_csv_data) {}
};

class QuotedSimpleDataFixture : public ExternalComparisonFixture {
public:
    QuotedSimpleDataFixture() : ExternalComparisonFixture(quoted_simple_csv_data) {}
};

// =============================================================
// csvengine "fastest" configuration (simple data):
//   ViewReader + SimdParser + MappedBuffer
// =============================================================
BENCHMARK_DEFINE_F(SimpleDataFixture, CSVREADER_COMP_CsvEngine_Fastest_Simple)
(benchmark::State& state) {
    Config cfg{
        .has_header    = true,
        .has_quoting   = false,
        .mapped_buffer = true,
        .use_simd      = true,
        .parse_mode    = Config::ParseMode::strict,
        .line_ending   = Config::LineEnding::lf,
    };

    std::size_t total_rows = 0;
    for (auto _ : state) {
        ViewReader reader(filename_, cfg);
        while (reader.next()) {
            auto rec = reader.current_record();
            benchmark::DoNotOptimize(rec);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}

// csvengine "owning Record" baseline (simple data):
//   Reader + SimpleParser + StreamBuffer
// =============================================================
BENCHMARK_DEFINE_F(SimpleDataFixture, CSVREADER_COMP_CsvEngine_OwningRecord_Simple)
(benchmark::State& state) {
    Config cfg{
        .has_header    = true,
        .has_quoting   = false,
        .mapped_buffer = false,
        .use_simd      = false,
        .line_ending   = Config::LineEnding::lf,
    };

    std::size_t total_rows = 0;
    for (auto _ : state) {
        Reader reader(filename_, cfg);
        while (reader.next()) {
            auto rec = reader.current_record();
            benchmark::DoNotOptimize(rec);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}

// csvengine quoted "fastest correct" configuration (FULL RFC 4180):
//   Reader + StrictQuotingParser + MappedBuffer
// =============================================================
BENCHMARK_DEFINE_F(QuotedDataFixture, CSVREADER_COMP_CsvEngine_Fastest_Quoted)
(benchmark::State& state) {
    Config cfg{
        .has_header    = true,
        .has_quoting   = true,
        .mapped_buffer = true,
        .parse_mode    = Config::ParseMode::strict,
        .line_ending   = Config::LineEnding::lf,
    };

    std::size_t total_rows = 0;
    for (auto _ : state) {
        Reader reader(filename_, cfg);
        while (reader.next()) {
            auto rec = reader.current_record();
            benchmark::DoNotOptimize(rec);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}

// csvengine quoted "fastest correct" configuration (subset):
//   Reader + StrictQuotingParser + MappedBuffer
// =============================================================
BENCHMARK_DEFINE_F(QuotedSimpleDataFixture, CSVREADER_COMP_CsvEngine_Fastest_QuotedSimple)
(benchmark::State& state) {
    Config cfg{
        .has_header    = true,
        .has_quoting   = true,
        .mapped_buffer = true,
        .parse_mode    = Config::ParseMode::strict,
        .line_ending   = Config::LineEnding::lf,
    };

    std::size_t total_rows = 0;
    for (auto _ : state) {
        Reader reader(filename_, cfg);
        while (reader.next()) {
            auto rec = reader.current_record();
            benchmark::DoNotOptimize(rec);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}

// =============================================================
// fast-cpp-csv-parser (Ben Strasser)
// =============================================================
#if defined(CSVENGINE_HAVE_FAST_CPP_CSV_PARSER)

// simple_csv_data has 3 columns: name, age, country
BENCHMARK_DEFINE_F(SimpleDataFixture, CSVREADER_COMP_FastCppCsvParser_Simple)
(benchmark::State& state) {
    std::size_t total_rows = 0;
    for (auto _ : state) {
        io::CSVReader<3, io::trim_chars<>, io::no_quote_escape<','>> in(filename_);
        in.read_header(io::ignore_no_column, "name", "age", "country");

        std::string name;
        std::string age;
        std::string country;
        while (in.read_row(name, age, country)) {
            benchmark::DoNotOptimize(name);
            benchmark::DoNotOptimize(age);
            benchmark::DoNotOptimize(country);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}

// NOTE: We don't have a fast-cpp-csv-parser counterpart for QuotedDataFixture
// because that dataset contains embedded newlines inside quoted fields
// (RFC 4180 §2.6), which fast-cpp-csv-parser does not support and throws
// `io::error::escaped_string_not_closed`.

// quoted_simple_csv_data has 3 columns: Product, Description, Price
// Fields may contain commas and escaped quotes, but NO embedded newlines.
BENCHMARK_DEFINE_F(QuotedSimpleDataFixture, CSVREADER_COMP_FastCppCsvParser_QuotedSimple)
(benchmark::State& state) {
    std::size_t total_rows = 0;
    for (auto _ : state) {
        io::CSVReader<3, io::trim_chars<>, io::double_quote_escape<',', '"'>> in(filename_);
        in.read_header(io::ignore_no_column, "Product", "Description", "Price");

        std::string product;
        std::string description;
        std::string price;
        while (in.read_row(product, description, price)) {
            benchmark::DoNotOptimize(product);
            benchmark::DoNotOptimize(description);
            benchmark::DoNotOptimize(price);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}

#endif // CSVENGINE_HAVE_FAST_CPP_CSV_PARSER

// =============================================================
// rapidcsv (d99kris)
// =============================================================
#if defined(CSVENGINE_HAVE_RAPIDCSV)

BENCHMARK_DEFINE_F(SimpleDataFixture, CSVREADER_COMP_RapidCsv_Simple)
(benchmark::State& state) {
    std::size_t total_rows = 0;
    for (auto _ : state) {
        // rapidcsv parses the ENTIRE file in its constructor.
        // LabelParams(0, -1) means row 0 is header, no column names.
        rapidcsv::Document doc(filename_, rapidcsv::LabelParams(0, -1));
        
        size_t row_count = doc.GetRowCount();
        for (size_t i = 0; i < row_count; ++i) {
            // Retrieve row as vector of strings
            auto row = doc.GetRow<std::string>(i);
            benchmark::DoNotOptimize(row);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}

// NOTE: rapidcsv struggles with embedded newlines in default configuration, 
// so we only benchmark it on QuotedSimpleDataFixture.
BENCHMARK_DEFINE_F(QuotedSimpleDataFixture, CSVREADER_COMP_RapidCsv_QuotedSimple)
(benchmark::State& state) {
    std::size_t total_rows = 0;
    for (auto _ : state) {
        // LabelParams(0, -1) means row 0 is header, no column names.
        // SeparatorParams defaults to ',' and handles standard quotes.
        rapidcsv::Document doc(filename_, rapidcsv::LabelParams(0, -1));
        
        size_t row_count = doc.GetRowCount();
        for (size_t i = 0; i < row_count; ++i) {
            auto row = doc.GetRow<std::string>(i);
            benchmark::DoNotOptimize(row);
            ++total_rows;
        }
        benchmark::DoNotOptimize(total_rows);
    }
    report(state, total_rows);
}


#endif // CSVENGINE_HAVE_RAPIDCSV

// =============================================================
// Registrations
// =============================================================

// --- Simple data ---
BENCHMARK_REGISTER_F(SimpleDataFixture, CSVREADER_COMP_CsvEngine_Fastest_Simple)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(SimpleDataFixture, CSVREADER_COMP_CsvEngine_OwningRecord_Simple)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);

#if defined(CSVENGINE_HAVE_FAST_CPP_CSV_PARSER)
BENCHMARK_REGISTER_F(SimpleDataFixture, CSVREADER_COMP_FastCppCsvParser_Simple)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);
#endif

#if defined(CSVENGINE_HAVE_RAPIDCSV)
BENCHMARK_REGISTER_F(SimpleDataFixture, CSVREADER_COMP_RapidCsv_Simple)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);
#endif

// --- Quoted data (FULL RFC 4180 with embedded \n) ---
BENCHMARK_REGISTER_F(QuotedDataFixture, CSVREADER_COMP_CsvEngine_Fastest_Quoted)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);

// --- Quoted data (Subset without embedded \n) ---
BENCHMARK_REGISTER_F(QuotedSimpleDataFixture, CSVREADER_COMP_CsvEngine_Fastest_QuotedSimple)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);

#if defined(CSVENGINE_HAVE_FAST_CPP_CSV_PARSER)
BENCHMARK_REGISTER_F(QuotedSimpleDataFixture, CSVREADER_COMP_FastCppCsvParser_QuotedSimple)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);
#endif

#if defined(CSVENGINE_HAVE_RAPIDCSV)
BENCHMARK_REGISTER_F(QuotedSimpleDataFixture, CSVREADER_COMP_RapidCsv_QuotedSimple)
    ->Arg(small_data)->Arg(medium_data)->Arg(big_data)->Arg(huge_data)
    ->Unit(benchmark::kMillisecond);
#endif

} // namespace external_bm
} // namespace csv
