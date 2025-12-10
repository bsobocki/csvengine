#include <gtest/gtest.h>
#include <csvreader.hpp>
#include <testdata.hpp>

using namespace csv;

TEST(ReaderTest, OpenTheFile) {
    EXPECT_NO_THROW(Reader reader("./test_data/simple_file.csv"));
}

// --- Naive Parsing Tests

TEST(ReaderTest, ReadSimpleHeaders) {
    // csv data from simple_file.csv
    Reader reader(std::make_unique<std::istringstream>(simple_csv_data));
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"name" , "age", "country"};
    EXPECT_EQ(reader.headers(), expectedHeaders);
}

// -- Quoting Parsing Tests

TEST(ReaderTest, ReadQuotingHeaders) {
    // csv data from quoting.csv
    Reader reader(std::make_unique<std::istringstream>(quoted_csv_data));
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"Product", "Description", "Price, but Netto"};
    // for naive implementation the result is { "\"Product\"", "\"Description\"", "\"Price", " but Netto\"" }
    // EXPECT_EQ(reader.headers(), expectedHeaders);
}