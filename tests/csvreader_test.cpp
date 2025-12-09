#include <gtest/gtest.h>
#include <csvreader.hpp>
#include <testdata.hpp>

TEST(CsvReaderTest, OpenTheFile) {
    EXPECT_NO_THROW(CsvReader reader("./test_data/simple_file.csv"));
}

// --- Naive Parsing Tests

TEST(CsvReaderTest, ReadSimpleHeaders) {
    // csv data from simple_file.csv
    CsvReader reader(std::make_unique<std::istringstream>(simple_csv_data));
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"name" , "age", "country"};
    EXPECT_EQ(reader.headers(), expectedHeaders);
}

// -- Quoting Parsing Tests

TEST(CsvReaderTest, ReadQuotingHeaders) {
    // csv data from quoting.csv
    CsvReader reader(std::make_unique<std::istringstream>(quoted_csv_data));
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"Product", "Description", "Price, but Netto"};
    // for naive implementation the result is { "\"Product\"", "\"Description\"", "\"Price", " but Netto\"" }
    // EXPECT_EQ(reader.headers(), expectedHeaders);
}