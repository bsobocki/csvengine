#include <gtest/gtest.h>
#include <csvbuffer/csvstreambuffer.hpp>
#include <testdata.hpp>
#include <string>

using namespace csv;
using namespace std::string_literals;

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

TEST(StreamBufferTest, DefaultStreamBuffer64KB_ReadSimpleFile) {
    using StreamBuffer64KB = StreamBuffer<>;

    constexpr size_t expected_refill_size = 137;
    StreamBuffer64KB buffer(std::make_unique<std::istringstream>(simple_csv_data));

    verify_buffer_chunk(buffer, simple_csv_data.data(), expected_refill_size);

    verify_eof(buffer);
}

TEST(StreamBufferTest, StreamBuffer40B_ReadSimpleFile_SeveralChunksUntilEof) {
    using StreamBuffer40B = StreamBuffer<40>;

    StreamBuffer40B buffer(std::make_unique<std::istringstream>(simple_csv_data));

    EXPECT_TRUE(buffer.good());

    verify_buffer_chunk(buffer, simple_csv_data.data(), 40);

    verify_buffer_chunk(buffer, simple_csv_data.data()+40, 40);

    verify_buffer_chunk(buffer, simple_csv_data.data()+80, 40);

    verify_buffer_chunk(buffer, simple_csv_data.data()+120, 17);

    verify_eof(buffer);
}

TEST(StreamBufferTest, DefaultStreamBuffer64KB_ReadEmptyFile) {
    using StreamBuffer64KB = StreamBuffer<>;

    StreamBuffer64KB buffer(std::make_unique<std::istringstream>(""));

    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.available(), expected_no_data);
    EXPECT_EQ(buffer.view(), std::string_view());
}

TEST(StreamBufferTest, DefaultStreamBuffer64KB_ReadFile_OneUnfilledChunkOnly) {
    using StreamBuffer64KB = StreamBuffer<>;
    const std::string data = "AAAAAA";

    StreamBuffer64KB buffer(std::make_unique<std::istringstream>(data));

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

TEST(StreamBufferTest, StreamBuffer64B_viewDoesNotConsume) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("test"));
    buffer.refill();
    
    auto data1 = buffer.view();
    auto data2 = buffer.view();
    
    EXPECT_EQ(data1, data2);
    EXPECT_EQ(buffer.available(), 4);
}

TEST(StreamBufferTest, StreamBuffer64B_consumeMoreThanAvailable) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("ABC"));
    buffer.refill();
    
    buffer.consume(100);
    EXPECT_EQ(buffer.available(), 0);
}

TEST(StreamBufferTest, StreamBuffer64B_Partialconsume) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));
    buffer.refill();
    
    buffer.consume(3);
    EXPECT_EQ(buffer.view(), "DEF");
    
    buffer.consume(2);
    EXPECT_EQ(buffer.view(), "F");
}

TEST(StreamBufferTest, StreamBuffer64B_PartialConsume) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 6);

    buffer.consume(3);
    EXPECT_EQ(buffer.available(), 3);

    EXPECT_EQ(buffer.view(), "DEF");

    buffer.consume(3);
    EXPECT_EQ(buffer.available(), expected_no_data);
}

TEST(StreamBufferTest, DefaultStreamBuffer64KB_ConsumeAllAndReset) {
    auto data = "ABCDEF";
    constexpr size_t data_size = 6;
    StreamBuffer<> buffer(std::make_unique<std::istringstream>(data));

    verify_buffer_chunk(buffer, data, data_size);
    buffer.reset();
    verify_buffer_chunk(buffer, data, data_size);

    verify_eof(buffer);
}

TEST(StreamBufferTest, StreamBuffer4_CompactMovesData) {
    auto data = "ABCDEF";
    StreamBuffer<4> buffer(std::make_unique<std::istringstream>(data));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "ABCD");
    buffer.consume(2);
    EXPECT_EQ(buffer.view(), "CD");
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "CDEF");
}

TEST(StreamBufferTests, StreamBuffer4_StreamBufferFullWithoutConsume) {
    StreamBuffer<4> buffer(std::make_unique<std::istringstream>("ABCDEFGH"));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);

    EXPECT_EQ(buffer.refill(), ReadingResult::buffer_full);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "ABCD");
}

TEST(StreamBufferTests, StreamBuffer64_ConsumeZeroBytes_EOF) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("ABC"));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 3);
    buffer.consume(0);

    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "ABC");

    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "ABC");
}

TEST(StreamBufferTests, StreamBuffer64_ConsumeZeroBytes_StreamBufferFull) {
    StreamBuffer<3> buffer(std::make_unique<std::istringstream>("ABCD"));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "ABC");
    buffer.consume(0);

    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "ABC");

    EXPECT_EQ(buffer.refill(), ReadingResult::buffer_full);
    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "ABC");
    buffer.consume(1);

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "BCD");
    
    EXPECT_EQ(buffer.refill(), ReadingResult::buffer_full);
    buffer.consume(3);
    
    verify_eof(buffer);
}

TEST(StreamBufferTests, DefaultStreamBuffer64K_EmptyBeforeFirstRefill) {
    StreamBuffer<3> buffer(std::make_unique<std::istringstream>("ABCD"));

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.available(), 0);
    EXPECT_EQ(buffer.view(), "");
}

TEST(StreamBufferTest, StreamBuffer64_MultipleRefillsAtEof) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("A"));
    
    buffer.refill();
    buffer.consume(1);
    
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
}

TEST(StreamBufferTest, StreamBuffer4_ExactFit) {
    StreamBuffer<4> buffer(std::make_unique<std::istringstream>("ABCD"));
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "ABCD");
    
    buffer.consume(4);
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
}

TEST(StreamBufferTest, StreamBuffer4_DataOneByteMoreThanStreamBuffer) {
    StreamBuffer<4> buffer(std::make_unique<std::istringstream>("ABCDE"));
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "ABCD");
    
    buffer.consume(4);
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "E");
}

TEST(StreamBufferTest, StreamBuffer4_CompactWithNoLeftover) {
    StreamBuffer<4> buffer(std::make_unique<std::istringstream>("ABCD"));
    
    buffer.refill();
    buffer.consume(4);
    
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.available(), 0);
}

TEST(StreamBufferTest, StreamBuffer4_CompactWithAllLeftover) {
    StreamBuffer<4> buffer(std::make_unique<std::istringstream>("ABCDEFGH"));
    
    buffer.refill();
    buffer.consume(0);
    
    EXPECT_EQ(buffer.refill(), ReadingResult::buffer_full);
    EXPECT_EQ(buffer.view(), "ABCD");
}

TEST(StreamBufferTest, StreamBuffer64_GoodStateTransitions) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("AB"));
    
    EXPECT_TRUE(buffer.good());
    EXPECT_FALSE(buffer.eof());
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_FALSE(buffer.eof());
    EXPECT_TRUE(buffer.good());

    buffer.consume(2);
    EXPECT_FALSE(buffer.good());
    
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_FALSE(buffer.good());
    EXPECT_TRUE(buffer.eof());
}

TEST(StreamBufferTest, StreamBuffer64_ResetBeforeAnyRead) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("ABC"));
    
    buffer.reset();
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "ABC");
}

TEST(StreamBufferTest, StreamBuffer64_ResetMidStream) {
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));
    
    buffer.refill();
    buffer.consume(3);
    EXPECT_EQ(buffer.view(), "DEF");
    
    buffer.reset();
    
    buffer.refill();
    EXPECT_EQ(buffer.view(), "ABCDEF");
}

TEST(StreamBufferTest, StreamBuffer1_SingleByteStreamBuffer) {
    StreamBuffer<1> buffer(std::make_unique<std::istringstream>("ABC"));
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "A");
    
    EXPECT_EQ(buffer.refill(), ReadingResult::buffer_full);
    
    buffer.consume(1);
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "B");
    
    buffer.consume(1);
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "C");
    
    buffer.consume(1);
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
}


TEST(StreamBufferTest, StreamBuffer24_ConsumeMoreThanCapacity) {
    StreamBuffer<24> buffer(std::make_unique<std::istringstream>("ABC"));

    buffer.consume(100);
    EXPECT_EQ(buffer.available(), 0);
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 3);
    
    buffer.consume(100);
    EXPECT_EQ(buffer.available(), 0);
}

TEST(StreamBufferTest, StreamBuffer64_BinaryDataWithNullBytes) {
    std::string data = "AB\0CD\0EF"s;
    StreamBuffer<64> buffer(std::make_unique<std::istringstream>(data));
    
    buffer.refill();
    EXPECT_EQ(buffer.available(), 8);
    EXPECT_EQ(buffer.view(), std::string_view(data));
}

TEST(StreamBufferTest, ThrowsWhenFileDoesNotExist) {
    EXPECT_THROW({
        csv::make_stream_buffer("/non_existent/path/to/file.csv");
    }, csv::FileStreamError);
}

TEST(StreamBufferTest, ThrowsOnInvalidIstream) {
    auto ss = std::make_unique<std::istringstream>("");

    ss->setstate(std::ios::failbit);

    EXPECT_THROW({
        csv::StreamBuffer<1024> buffer(std::move(ss));
    }, csv::FileStreamError);
}
