#include <gtest/gtest.h>
#include <csvreader.hpp>
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

    TestSetup setup_reader(Config cfg = {}) {
        auto mock_buffer = std::make_unique<MockBuffer>();
        MockBuffer* mock_buffer_ptr = mock_buffer.get();

        // 1. Set default behavior that most tests will use
        ON_CALL(*mock_buffer_ptr, good()).WillByDefault(Return(true));

        // 2. If the config says there's a header, we MUST set expectations 
        // because the constructor will call read_headers() immediately.
        if (cfg.has_header) {
            EXPECT_CALL(*mock_buffer_ptr, empty()).WillOnce(Return(false));
            EXPECT_CALL(*mock_buffer_ptr, view()).WillOnce(Return("h1,h2\n"));
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
    EXPECT_CALL(*mock, empty()).WillOnce(Return(true));
    EXPECT_CALL(*mock, refill()).WillOnce(Return(ReadingResult::eof));

    EXPECT_EQ(reader->headers(), std::vector<std::string>({"h1", "h2"}));

    EXPECT_FALSE(reader->next());
    
    EXPECT_EQ(reader->current_record().fields(), std::vector<std::string>({"h1", "h2"}));
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