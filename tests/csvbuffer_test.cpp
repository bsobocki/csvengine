#include <gtest/gtest.h>
#include <csvbuffer.hpp>
#include <testdata.hpp>

constexpr size_t expected_no_data = 0;

void verify_buffer_chunk(auto& buffer, const char* expected_start, size_t expected_size) {
    using BufferType = std::remove_reference_t<decltype(buffer)>;

    EXPECT_TRUE(buffer.good());

    EXPECT_EQ(buffer.read_data(), BufferType::ReadingResult::ok);
    EXPECT_EQ(buffer.available_data_size(), expected_size);

    auto expected_data = std::string_view(expected_start, expected_size);
    EXPECT_EQ(buffer.peek(), expected_data);
    EXPECT_EQ(buffer.available_data_size(), expected_size);
    EXPECT_EQ(buffer.consume_available_bytes(), expected_data);

    EXPECT_EQ(buffer.available_data_size(), expected_no_data);
    EXPECT_EQ(buffer.peek(), std::string_view());
    EXPECT_EQ(buffer.consume_available_bytes(), std::string_view());
}

void verify_eof(auto& buffer) {
    using BufferType = std::remove_reference_t<decltype(buffer)>;
    EXPECT_EQ(buffer.read_data(), BufferType::ReadingResult::eof);
    EXPECT_TRUE(buffer.eof());
}

TEST(CsvBufferTest, DefaultBuffer64KB_ReadSimpleFile) {
    using CsvBuffer64KB = CsvBuffer<>;

    constexpr size_t expected_read_data_size = 137;
    CsvBuffer64KB buffer(std::make_unique<std::istringstream>(simple_csv_data));

    // read 137 bytes
    verify_buffer_chunk(buffer, simple_csv_data.data(), expected_read_data_size);

    // hit eof
    verify_eof(buffer);
}

TEST(CsvBufferTest, Buffer40B_ReadSimpleFile_SeveralChunksUntilEof) {
    using CsvBuffer40B = CsvBuffer<40>;

    CsvBuffer40B buffer(std::make_unique<std::istringstream>(simple_csv_data));

    EXPECT_TRUE(buffer.good());

    // read 1-40 bytes
    verify_buffer_chunk(buffer, simple_csv_data.data(), 40);
    
    // read 41-80 bytes
    verify_buffer_chunk(buffer, simple_csv_data.data()+40, 40);

    // read 81-120 bytes
    verify_buffer_chunk(buffer, simple_csv_data.data()+80, 40);
    
    // read 121-137 bytes
    verify_buffer_chunk(buffer, simple_csv_data.data()+120, 17);

    // hit eof
    verify_eof(buffer);
}

TEST(CsvBufferTest, DefaultBuffer64KB_ReadEmptyFile) {
    using CsvBuffer64KB = CsvBuffer<>;

    CsvBuffer64KB buffer(std::make_unique<std::istringstream>(""));

    EXPECT_EQ(buffer.read_data(), CsvBuffer64KB::ReadingResult::eof);
    EXPECT_EQ(buffer.available_data_size(), expected_no_data);
    EXPECT_EQ(buffer.peek(), std::string_view());
}

TEST(CsvBufferTest, DefaultBuffer64KB_ReadFile_OneUnfilledChunkOnly) {
    using CsvBuffer64KB = CsvBuffer<>;
    const std::string data = "AAAAAA";

    CsvBuffer64KB buffer(std::make_unique<std::istringstream>(data));

    EXPECT_EQ(buffer.read_data(), CsvBuffer64KB::ReadingResult::ok);
    EXPECT_EQ(buffer.available_data_size(), data.size());
    EXPECT_EQ(buffer.peek(), std::string_view(data));
    EXPECT_EQ(buffer.peek(2), std::string_view(data.data(), 2));

    buffer.shift(2);
    EXPECT_EQ(buffer.consume_available_bytes(), std::string_view(data.data()+2));
    EXPECT_EQ(buffer.available_data_size(), expected_no_data);
    
    EXPECT_EQ(buffer.read_data(), CsvBuffer64KB::ReadingResult::eof);
    EXPECT_EQ(buffer.available_data_size(), expected_no_data);
    EXPECT_EQ(buffer.peek(), std::string_view());
}

TEST(CsvBufferTest, Buffer64B_PeekDoesNotConsume) {
    CsvBuffer<64> buffer(std::make_unique<std::istringstream>("test"));
    buffer.read_data();
    
    auto data1 = buffer.peek();
    auto data2 = buffer.peek();
    
    EXPECT_EQ(data1, data2);
    EXPECT_EQ(buffer.available_data_size(), 4);
}

TEST(CsvBufferTest, Buffer64B_ShiftMoreThanAvailable) {
    CsvBuffer<64> buffer(std::make_unique<std::istringstream>("ABC"));
    buffer.read_data();
    
    buffer.shift(100);
    EXPECT_EQ(buffer.available_data_size(), 0);
}

TEST(CsvBufferTest, Buffer64B_PartialShift) {
    CsvBuffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));
    buffer.read_data();
    
    buffer.shift(3);
    EXPECT_EQ(buffer.peek(), "DEF");
    
    buffer.shift(2);
    EXPECT_EQ(buffer.peek(), "F");
}

TEST(CsvBufferTest, Buffer64B_PartialConsume) {
    CsvBuffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));

    EXPECT_EQ(buffer.read_data(), CsvBuffer<64>::ReadingResult::ok);
    EXPECT_EQ(buffer.available_data_size(), 6);

    EXPECT_EQ(buffer.consume_bytes(3), "ABC");
    EXPECT_EQ(buffer.available_data_size(), 3);

    EXPECT_EQ(buffer.peek(), "DEF");

    EXPECT_EQ(buffer.consume_bytes(3), "DEF");
    EXPECT_EQ(buffer.available_data_size(), expected_no_data);
}

TEST(CsvBufferTest, DefaultBuffer64KB_ConsumeAllAndReset) {
    auto data = "ABCDEF";
    constexpr size_t data_size = 6;
    CsvBuffer<> buffer(std::make_unique<std::istringstream>(data));

    verify_buffer_chunk(buffer, data, data_size);
    buffer.reset();
    verify_buffer_chunk(buffer, data, data_size);

    verify_eof(buffer);
}