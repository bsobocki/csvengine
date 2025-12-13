#include <gtest/gtest.h>
#include <csvbuffer.hpp>
#include <testdata.hpp>

using namespace csv;

constexpr size_t expected_no_data = 0;

void verify_buffer_chunk(auto& buffer, const char* expected_start, size_t expected_size) {
    EXPECT_TRUE(buffer.good());

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), expected_size);

    auto expected_data = std::string_view(expected_start, expected_size);
    EXPECT_EQ(buffer.view(), expected_data);
    EXPECT_EQ(buffer.available(), expected_size);
    buffer.consume(buffer.available());

    EXPECT_EQ(buffer.available(), expected_no_data);
    EXPECT_EQ(buffer.view(), std::string_view());
}

void verify_eof(auto& buffer) {
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_TRUE(buffer.eof());
}

TEST(BufferTest, DefaultBuffer64KB_ReadSimpleFile) {
    using Buffer64KB = Buffer<>;

    constexpr size_t expected_refill_size = 137;
    Buffer64KB buffer(std::make_unique<std::istringstream>(simple_csv_data));

    // read 137 bytes
    verify_buffer_chunk(buffer, simple_csv_data.data(), expected_refill_size);

    // hit eof
    verify_eof(buffer);
}

TEST(BufferTest, Buffer40B_ReadSimpleFile_SeveralChunksUntilEof) {
    using Buffer40B = Buffer<40>;

    Buffer40B buffer(std::make_unique<std::istringstream>(simple_csv_data));

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

TEST(BufferTest, DefaultBuffer64KB_ReadEmptyFile) {
    using Buffer64KB = Buffer<>;

    Buffer64KB buffer(std::make_unique<std::istringstream>(""));

    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.available(), expected_no_data);
    EXPECT_EQ(buffer.view(), std::string_view());
}

TEST(BufferTest, DefaultBuffer64KB_ReadFile_OneUnfilledChunkOnly) {
    using Buffer64KB = Buffer<>;
    const std::string data = "AAAAAA";

    Buffer64KB buffer(std::make_unique<std::istringstream>(data));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), data.size());
    EXPECT_EQ(buffer.view(), std::string_view(data));

    buffer.consume(2);
    EXPECT_EQ(buffer.view(), std::string_view(data.data()+2));
    EXPECT_EQ(buffer.available(), data.size()-2);
    
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.available(), data.size()-2);
    EXPECT_EQ(buffer.view(), std::string_view(data.data()+2));
}

TEST(BufferTest, Buffer64B_viewDoesNotConsume) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("test"));
    buffer.refill();
    
    auto data1 = buffer.view();
    auto data2 = buffer.view();
    
    EXPECT_EQ(data1, data2);
    EXPECT_EQ(buffer.available(), 4);
}

TEST(BufferTest, Buffer64B_consumeMoreThanAvailable) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("ABC"));
    buffer.refill();
    
    buffer.consume(100);
    EXPECT_EQ(buffer.available(), 0);
}

TEST(BufferTest, Buffer64B_Partialconsume) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));
    buffer.refill();
    
    buffer.consume(3);
    EXPECT_EQ(buffer.view(), "DEF");
    
    buffer.consume(2);
    EXPECT_EQ(buffer.view(), "F");
}

TEST(BufferTest, Buffer64B_PartialConsume) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 6);

    buffer.consume(3);
    EXPECT_EQ(buffer.available(), 3);

    EXPECT_EQ(buffer.view(), "DEF");

    buffer.consume(3);
    EXPECT_EQ(buffer.available(), expected_no_data);
}

TEST(BufferTest, DefaultBuffer64KB_ConsumeAllAndReset) {
    auto data = "ABCDEF";
    constexpr size_t data_size = 6;
    Buffer<> buffer(std::make_unique<std::istringstream>(data));

    verify_buffer_chunk(buffer, data, data_size);
    buffer.reset();
    verify_buffer_chunk(buffer, data, data_size);

    verify_eof(buffer);
}