#include <gtest/gtest.h>
#include <csvreader.hpp>
#include <csvbuffer/csvstreambuffer.hpp>
#include <sstream>

using namespace csv;
using namespace std;

class ReaderRecordViewTest : public ::testing::Test {
protected:
    template <size_t N>
    ReaderRecordView createReader(const std::string& data, Config cfg = {}) {
        auto stream = std::make_unique<std::stringstream>(data);
        auto buffer = std::make_unique<StreamBuffer<N>>(std::move(stream));
        return ReaderRecordView(std::move(buffer), cfg);
    }
};

TEST_F(ReaderRecordViewTest, ReadsSimpleDataCorrectly) {
    // Buffer duży, Happy Path
    auto reader = createReader<1024>("col1,col2\nval1,val2\n");
    
    // Header (czytany w init)
    auto headers = reader.headers();
    ASSERT_EQ(headers.size(), 2);
    EXPECT_EQ(headers[0], "col1");
    EXPECT_EQ(headers[1], "col2");

    // Rekord 1
    ASSERT_TRUE(reader.next());
    auto rec = reader.current_record();
    EXPECT_EQ(rec.size(), 2);       // Tu był błąd "Got 4"
    EXPECT_EQ(rec[0], "val1");
    EXPECT_EQ(rec[1], "val2");
    
    ASSERT_FALSE(reader.next());
}

TEST_F(ReaderRecordViewTest, RecordLargerThanBuffer_ThrowsException) {
    auto reader = createReader<4>("aa,bb\ncc,dd", {.has_header = false});
    
    EXPECT_THROW(reader.next(), std::runtime_error);
}

TEST_F(ReaderRecordViewTest, SplitRecord_FitsInBuffer_StitchingWorks) {
    auto reader = createReader<4>("a\nb\nc\n", {.has_header = false});
    ASSERT_TRUE(reader.next()); EXPECT_EQ(reader.current_record()[0], "a");
    ASSERT_TRUE(reader.next()); EXPECT_EQ(reader.current_record()[0], "b");
    ASSERT_TRUE(reader.next()); EXPECT_EQ(reader.current_record()[0], "c");
}

TEST_F(ReaderRecordViewTest, NoNewlineAtEnd) {
    // "a,b" -> bez \n
    auto reader = createReader<10>("a,b", {.has_header = false});
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.current_record()[0], "a");
    EXPECT_EQ(reader.current_record()[1], "b");
    ASSERT_FALSE(reader.next());
}

TEST_F(ReaderRecordViewTest, EmptyLines) {
    auto reader = createReader<10>("a\n\nb", {.has_header = false});
    
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.current_record()[0], "a");

    EXPECT_THROW(reader.next(), RecordSizeError);
}

TEST_F(ReaderRecordViewTest, FieldsAreContiguousInMemory) {
    auto reader = createReader<100>("12,34", {.has_header = false});
    ASSERT_TRUE(reader.next());
    auto rec = reader.current_record();
    
    const char* ptr1 = rec[0].data();
    const char* ptr2 = rec[1].data();

    EXPECT_EQ(ptr1 + 3, ptr2);
}

TEST_F(ReaderRecordViewTest, HeadersAreSaved_NextFieldsAreCorrectlyRead) {
    auto reader = createReader<100>("AA,BB\n12,34\n56,78\n", {.has_header = true});
    EXPECT_TRUE(reader.has_header());
    EXPECT_EQ(reader.headers(), std::vector<std::string>({"AA", "BB"}));

    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.current_record().fields(), std::vector<std::string_view>({"12", "34"}));
    EXPECT_EQ(reader.headers(), std::vector<std::string>({"AA", "BB"}));

    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.current_record().fields(), std::vector<std::string_view>({"56", "78"}));
    EXPECT_EQ(reader.headers(), std::vector<std::string>({"AA", "BB"}));
}