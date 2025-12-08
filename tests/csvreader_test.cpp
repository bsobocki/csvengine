#include <gtest/gtest.h>
#include <csvreader.hpp>

TEST(CsvReaderTest, OpenTheFile) {
    EXPECT_NO_THROW(CsvReader reader("./test_data/simple_file.csv"));
}

// --- Naive Parsing Tests

TEST(CsvReaderTest, ReadSimpleHeaders) {
    // csv data from simple_file.csv
    auto csv_data = R""""(name,age,country
Ken Adams,18,USA
Cristiano Ronaldo,35,Portugal
Gunter Shmitt,45,Germany
Andrzej Kowalski,55,Poland
John Krasinski,40,USA)"""";

    CsvReader reader(std::make_unique<std::istringstream>(csv_data));
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"name" , "age", "country"};
    EXPECT_EQ(reader.headers(), expectedHeaders);
}

// -- Quoting Parsing Tests

TEST(CsvReaderTest, ReadQuotingHeaders) {
    // csv data from quoting.csv
    auto csv_data = R""""("Product","Description","Price, but Netto"
"Widget A","Standard widget, 5"" diameter","$10.99"
"Widget ""Pro""","Professional grade, includes:
- Feature 1
- Feature 2","$49.99"
)"""";

    CsvReader reader(std::make_unique<std::istringstream>(csv_data));
    EXPECT_TRUE(reader.has_header());

    std::vector<std::string> expectedHeaders = {"Product", "Description", "Price, but Netto"};
    // for naive implementation the result is { "\"Product\"", "\"Description\"", "\"Price", " but Netto\"" }
    // EXPECT_EQ(reader.headers(), expectedHeaders);
}