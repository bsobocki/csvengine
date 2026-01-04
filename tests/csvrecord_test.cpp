#include <csvrecord.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace csv;

using str_vec = std::vector<std::string>;

TEST(RecordTest, CreateEmptyRecord_NoThrow) {
    EXPECT_NO_THROW(Record record);
}

TEST(RecordTest, EmptyRecord_NulloptOnGet) {
    Record record;
    EXPECT_EQ(record.get<int>(0), std::nullopt);
    EXPECT_EQ(record.get<int>(10), std::nullopt);
    EXPECT_EQ(record.get<int>("name"), std::nullopt);
    EXPECT_EQ(record.get<int>(""), std::nullopt);
}

TEST(RecordTest, NormalRecord_GetIntValues) {
    Record record(str_vec{"10", "20", "30"});
    EXPECT_EQ(record.get<int>(0), 10);
    EXPECT_EQ(record.get<int>(1), 20);
    EXPECT_EQ(record.get<int>(2), 30);
}

TEST(RecordTest, NormalRecord_NulloptOnGetWithInvalidIndex) {
    Record record(str_vec{"10", "20", "30"});
    EXPECT_EQ(record.get<int>(4), std::nullopt);
    EXPECT_EQ(record.get<int>(10), std::nullopt);
}