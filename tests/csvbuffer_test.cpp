#include <gtest/gtest.h>
#include <csvbuffer.hpp>
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

TEST(BufferTest, Buffer4_CompactMovesData) {
    auto data = "ABCDEF";
    Buffer<4> buffer(std::make_unique<std::istringstream>(data));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "ABCD");
    buffer.consume(2);
    EXPECT_EQ(buffer.view(), "CD");
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "CDEF");
}

TEST(BufferTests, Buffer4_BufferFullWithoutConsume) {
    Buffer<4> buffer(std::make_unique<std::istringstream>("ABCDEFGH"));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);

    EXPECT_EQ(buffer.refill(), ReadingResult::buffer_full);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "ABCD");
}

TEST(BufferTests, Buffer64_ConsumeZeroBytes_EOF) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("ABC"));

    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 3);
    buffer.consume(0);

    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "ABC");

    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.available(), 3);
    EXPECT_EQ(buffer.view(), "ABC");
}

TEST(BufferTests, Buffer64_ConsumeZeroBytes_BufferFull) {
    Buffer<3> buffer(std::make_unique<std::istringstream>("ABCD"));

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

TEST(BufferTests, DefaultBuffer64K_EmptyBeforeFirstRefill) {
    Buffer<3> buffer(std::make_unique<std::istringstream>("ABCD"));

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.available(), 0);
    EXPECT_EQ(buffer.view(), "");
}

TEST(BufferTest, Buffer64_MultipleRefillsAtEof) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("A"));
    
    buffer.refill();
    buffer.consume(1);
    
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
}

TEST(BufferTest, Buffer4_ExactFit) {
    Buffer<4> buffer(std::make_unique<std::istringstream>("ABCD"));
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 4);
    EXPECT_EQ(buffer.view(), "ABCD");
    
    buffer.consume(4);
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
}

TEST(BufferTest, Buffer4_DataOneByteMoreThanBuffer) {
    Buffer<4> buffer(std::make_unique<std::istringstream>("ABCDE"));
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "ABCD");
    
    buffer.consume(4);
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "E");
}

TEST(BufferTest, Buffer4_CompactWithNoLeftover) {
    Buffer<4> buffer(std::make_unique<std::istringstream>("ABCD"));
    
    buffer.refill();
    buffer.consume(4);  // consume all
    
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);
    EXPECT_EQ(buffer.available(), 0);
}

TEST(BufferTest, Buffer4_CompactWithAllLeftover) {
    Buffer<4> buffer(std::make_unique<std::istringstream>("ABCDEFGH"));
    
    buffer.refill();
    // Don't consume anything, compact should be no-op
    buffer.consume(0);
    
    EXPECT_EQ(buffer.refill(), ReadingResult::buffer_full);
    EXPECT_EQ(buffer.view(), "ABCD");
}

TEST(BufferTest, Buffer64_GoodStateTransitions) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("AB"));
    
    EXPECT_TRUE(buffer.good());
    EXPECT_FALSE(buffer.eof());
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_FALSE(buffer.eof()); // stream at eof, but not checked yet in refill
    EXPECT_TRUE(buffer.good()); // stream good and buffer not empty

    buffer.consume(2);
    EXPECT_FALSE(buffer.good()); // empty buffer
    
    EXPECT_EQ(buffer.refill(), ReadingResult::eof);  // now hits eof
    EXPECT_FALSE(buffer.good());
    EXPECT_TRUE(buffer.eof());
}

TEST(BufferTest, Buffer64_ResetBeforeAnyRead) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("ABC"));
    
    buffer.reset();  // reset without any operations
    
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.view(), "ABC");
}

TEST(BufferTest, Buffer64_ResetMidStream) {
    Buffer<64> buffer(std::make_unique<std::istringstream>("ABCDEF"));
    
    buffer.refill();
    buffer.consume(3);
    EXPECT_EQ(buffer.view(), "DEF");
    
    buffer.reset();
    
    buffer.refill();
    EXPECT_EQ(buffer.view(), "ABCDEF");  // back to start
}

TEST(BufferTest, Buffer1_SingleByteBuffer) {
    Buffer<1> buffer(std::make_unique<std::istringstream>("ABC"));
    
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


TEST(BufferTest, Buffer24_ConsumeMoreThanCapacity) {
    Buffer<24> buffer(std::make_unique<std::istringstream>("ABC"));

    buffer.consume(100);
    EXPECT_EQ(buffer.available(), 0);
    EXPECT_EQ(buffer.refill(), ReadingResult::ok);
    EXPECT_EQ(buffer.available(), 3);
    
    buffer.consume(100);
    EXPECT_EQ(buffer.available(), 0);
}

TEST(BufferTest, Buffer64_BinaryDataWithNullBytes) {
    std::string data = "AB\0CD\0EF"s;  // nulls in string literal
    Buffer<64> buffer(std::make_unique<std::istringstream>(data));
    
    buffer.refill();
    EXPECT_EQ(buffer.available(), 8);
    EXPECT_EQ(buffer.view(), std::string_view(data));
}

TEST(BufferTest, ThrowsWhenFileDoesNotExist) {
    EXPECT_THROW({
        csv::make_buffer("/non_existent/path/to/file.csv");
    }, csv::FileStreamError);
}

TEST(BufferTest, ThrowsOnInvalidIstream) {
    auto ss = std::make_unique<std::istringstream>("");
    
    // force istream into a 'fail' state
    ss->setstate(std::ios::failbit);

    EXPECT_THROW({
        csv::Buffer<1024> buffer(std::move(ss));
    }, csv::FileStreamError);
}
