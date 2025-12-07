#include <gtest/gtest.h>
#include <csvreader.hpp>

TEST(CsvReaderTest, OpenTheFile) {
    EXPECT_NO_THROW(CsvReader reader("./test_data/simple_file.csv"));
}

TEST(CsvReaderTest, ReadHeaders) {
    CsvReader reader("./test_data/simple_file.csv");
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"name" , "age", "country"};
    EXPECT_EQ(reader.headers(), expectedHeaders);
}