#include <gtest/gtest.h>
#include <csvreader.hpp>

TEST(CsvReaderTest, OpenTheFile) {
    EXPECT_NO_THROW(CsvReader reader("./test_data/simple_file.csv"));
}

TEST(CsvReaderTest, ReadSimpleHeaders) {
    CsvReader reader("./test_data/simple_file.csv");
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"name" , "age", "country"};
    EXPECT_EQ(reader.headers(), expectedHeaders);
}

TEST(CsvReaderTest, ReadQuotingHeaders) {
    CsvReader reader("./test_data/quoting.csv");
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"Product", "Description", "Price, but Netto"};
    // for naive implementation the result is { "\"Product\"", "\"Description\"", "\"Price", " but Netto\"" }
    EXPECT_EQ(reader.headers(), expectedHeaders);
}