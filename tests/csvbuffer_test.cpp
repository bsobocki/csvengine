#include <gtest/gtest.h>
#include <csvbuffer.hpp>

TEST(CsvBufferTest, BufferReadsSimpleFile) {
    // csv data from simple_file.csv
    auto csv_data = R""""(name,age,country
Ken Adams,18,USA
Cristiano Ronaldo,35,Portugal
Gunter Shmitt,45,Germany
Andrzej Kowalski,55,Poland
John Krasinski,40,USA)"""";

    CsvBuffer<> buffer(std::make_unique<std::istringstream>(csv_data));

    EXPECT_TRUE(buffer.good());

    // read 137 bytes
    EXPECT_EQ(buffer.read_data(), CsvBuffer<>::ReadingResult::ok);

    size_t expected_read_data_size = 137;
    EXPECT_EQ(buffer.available_data_size(), expected_read_data_size);

    // hit eof
    EXPECT_EQ(buffer.read_data(), CsvBuffer<>::ReadingResult::eof);
}

TEST(CsvBufferTest, BufferReadsQuotingFile) {
    
}