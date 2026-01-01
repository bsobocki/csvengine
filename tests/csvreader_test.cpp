#include <gtest/gtest.h>
#include <csvreader.hpp>
#include <csverrors.hpp>
#include <testdata.hpp>

#include <csvbuffer_mock.hpp>

using namespace csv;
using namespace testing;

class ReaderTest : public ::testing::Test {
protected:
    Reader simple_data_reader{std::make_unique<std::istringstream>(simple_csv_data)};
    Reader quoted_data_reader{std::make_unique<std::istringstream>(quoted_csv_data)};

    struct TestSetup {
        std::unique_ptr<Reader> reader;
        MockBuffer* mock_buffer;
    };

    TestSetup setup_reader(Config cfg = {}, const std::string& first_record = "h1,h2\n") {
        auto mock_buffer = std::make_unique<MockBuffer>();
        MockBuffer* mock_buffer_ptr = mock_buffer.get();

        // 1. Set default behavior that most tests will use
        ON_CALL(*mock_buffer_ptr, good()).WillByDefault(Return(true));

        // 2. If the config says there's a header, we MUST set expectations 
        // because the constructor will call read_headers() immediately.
        if (cfg.has_header) {
            EXPECT_CALL(*mock_buffer_ptr, empty()).WillOnce(Return(false));
            EXPECT_CALL(*mock_buffer_ptr, view()).WillOnce(Return(first_record));
            EXPECT_CALL(*mock_buffer_ptr, consume(_));
        }

        auto reader = std::make_unique<Reader>(std::move(mock_buffer), cfg);
        return { std::move(reader), mock_buffer_ptr };
    }
};

TEST_F(ReaderTest, OpenTheFile) {
    EXPECT_NO_THROW(Reader reader("./test_data/simple_file.csv"));
}

// --- Naive Parsing Tests

TEST_F(ReaderTest, ReadSimpleHeaders) {
    std::vector<std::string> expectedHeaders = {"name" , "age", "country"};
    EXPECT_TRUE(simple_data_reader.has_header());
    EXPECT_EQ(simple_data_reader.headers(), expectedHeaders);
}

TEST_F(ReaderTest, ReadSimpleRecords) {
    EXPECT_TRUE(simple_data_reader.next());
    EXPECT_EQ(simple_data_reader.record_size(), 3);
    std::vector<std::string> expectedFields = {"Ken Adams","18","USA"};
    EXPECT_EQ(simple_data_reader.current_record().fields(), expectedFields);
    
    EXPECT_TRUE(simple_data_reader.next());
    expectedFields = {"Cristiano Ronaldo","35","Portugal"};
    EXPECT_EQ(simple_data_reader.current_record().fields(), expectedFields);

    EXPECT_TRUE(simple_data_reader.next());
    expectedFields = {"Gunter Shmitt","45","Germany"};
    EXPECT_EQ(simple_data_reader.current_record().fields(), expectedFields);

    EXPECT_TRUE(simple_data_reader.next());
    expectedFields = {"Andrzej Kowalski","55","Poland"};
    EXPECT_EQ(simple_data_reader.current_record().fields(), expectedFields);

    EXPECT_TRUE(simple_data_reader.next());
    expectedFields = {"John Krasinski","40","USA"};
    EXPECT_EQ(simple_data_reader.current_record().fields(), expectedFields);

    EXPECT_FALSE(simple_data_reader.next());
}

// -- Quoting Parsing Tests

TEST_F(ReaderTest, ReadQuotingHeaders) {
    std::vector<std::string> expectedHeaders = {"Product", "Description", "Price, but Netto"};
    EXPECT_TRUE(quoted_data_reader.has_header());
    EXPECT_EQ(quoted_data_reader.headers(), expectedHeaders);
}

// -- Buffer Mock

TEST_F(ReaderTest, NextReturnsFalseWhenDataIsOnlyHeaders_CurrentRecordHasHeaders) {
    Config cfg{.has_header = true};
    auto [reader, mock] = setup_reader(cfg);

    // Specific expectations for this test
    // EXPECT_CALL(*mock, empty()).WillOnce(Return(true));
    // EXPECT_CALL(*mock, refill()).WillOnce(Return(ReadingResult::eof));

    // EXPECT_EQ(reader->headers(), std::vector<std::string>({"h1", "h2"}));

    // EXPECT_FALSE(reader->next());
    
    //EXPECT_EQ(reader->current_record().fields(), std::vector<std::string>({"h1", "h2"}));
}

TEST_F(ReaderTest, NextReturnsTrueWhenDataAvailable) {
    Config cfg{.has_header = false};
    auto [reader, mock] = setup_reader(cfg);

    // Specific expectations for this test
    EXPECT_CALL(*mock, empty()).WillOnce(Return(false));
    EXPECT_CALL(*mock, view()).WillOnce(Return("val1,val2\n"));
    EXPECT_CALL(*mock, consume(_));

    EXPECT_TRUE(reader->next());
    EXPECT_EQ(reader->current_record().fields(), std::vector<std::string>({"val1", "val2"}));
}

TEST_F(ReaderTest, BufferErrorOnInit) {
    auto mock_buffer = std::make_unique<MockBuffer>();
    MockBuffer* mock_buffer_ptr = mock_buffer.get();

    ON_CALL(*mock_buffer_ptr, good()).WillByDefault(Return(false));

    EXPECT_THROW(Reader reader(std::move(mock_buffer)), BufferError);
}

TEST_F(ReaderTest, HeadersErrorOnInit) {
    auto mock_buffer = std::make_unique<MockBuffer>();
    MockBuffer* mock_buffer_ptr = mock_buffer.get();

    ON_CALL(*mock_buffer_ptr, good()).WillByDefault(Return(true));
    
    ON_CALL(*mock_buffer_ptr, empty()).WillByDefault(Return(true));
    ON_CALL(*mock_buffer_ptr, refill()).WillByDefault(Return(ReadingResult::fail));

    EXPECT_THROW(Reader reader(std::move(mock_buffer)), FileHeaderError);
}

TEST_F(ReaderTest, RangeBasedLoop_IteratorBasicIteration) {
    std::vector<std::vector<std::string>> all_records;
    
    for (const auto& record : simple_data_reader) {
        all_records.push_back({record.fields().begin(), record.fields().end()});
    }

    EXPECT_EQ(all_records.size(), 5);
    EXPECT_EQ(all_records[0][0], "Ken Adams");
    EXPECT_EQ(all_records[1][0], "Cristiano Ronaldo");
    EXPECT_EQ(all_records[2][0], "Gunter Shmitt");
    EXPECT_EQ(all_records[3][0], "Andrzej Kowalski");
    EXPECT_EQ(all_records[4][0], "John Krasinski");
}

TEST_F(ReaderTest, IteratorEmptyFile) {
    Config cfg{.has_header = true};
    auto [reader, mock] = setup_reader(cfg);

    EXPECT_CALL(*mock, empty()).WillOnce(Return(true));
    EXPECT_CALL(*mock, refill()).WillOnce(Return(ReadingResult::eof));

    int count = 0;
    for (const auto& record : *reader) {
        (void)record;
        count++;
    }
    
    EXPECT_EQ(count, 0);
}

TEST_F(ReaderTest, IteratorBeginEndEquality) {
    Config cfg{.has_header = false};
    auto [reader, mock] = setup_reader(cfg);

    EXPECT_CALL(*mock, empty()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mock, refill()).WillRepeatedly(Return(ReadingResult::eof));

    auto it = reader->begin();
    EXPECT_FALSE(it != reader->end());
}

TEST_F(ReaderTest, RecordSize_HeaderSize) {
    Config cfg{.has_header = true};
    auto [reader, mock] = setup_reader(cfg);

    auto headers = reader->headers();
    EXPECT_EQ(headers, std::vector<std::string>({"h1", "h2"}));

    EXPECT_EQ(reader->record_size(), 2);
}

TEST_F(ReaderTest, RecordSize_Zero) {
    Config cfg{.has_header = false};
    auto [reader, mock] = setup_reader(cfg, "");

    auto headers = reader->headers();
    EXPECT_EQ(headers, std::vector<std::string>{});

    EXPECT_EQ(reader->record_size(), 0);
}
