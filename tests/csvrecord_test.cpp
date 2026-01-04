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

TEST(RecordTest, NormalRecord_GetStringValues) {
    Record record(str_vec{"10", "20", "30", "mamma mia!", "   alehandro!  "});
    EXPECT_EQ(record.get<std::string>(0), "10");
    EXPECT_EQ(record.get<std::string>(1), "20");
    EXPECT_EQ(record.get<std::string>(2), "30");
    EXPECT_EQ(record.get<std::string>(3), "mamma mia!");
    EXPECT_EQ(record.get<std::string>(4), "   alehandro!  ");
}

TEST(RecordTest, NormalRecord_GetDoubleValues) {
    Record record(str_vec{"10", "2.50", "3.506"});
    EXPECT_EQ(record.get<double>(0), 10.0);
    EXPECT_EQ(record.get<double>(1), 2.5);
    EXPECT_EQ(record.get<double>(2), 3.506);
}

TEST(RecordTest, NormalRecord_NulloptOnGetWithInvalidIndex) {
    Record record(str_vec{"10", "20", "30"});
    EXPECT_EQ(record.get<int>(4), std::nullopt);
    EXPECT_EQ(record.get<int>(10), std::nullopt);
}

TEST(RecordTest, NormalRecord_GetValues_TrailingSpacesRemovedForArithmeticsValues) {
    Record record(str_vec{"  10   ", " 20 ", "   30      "});
    EXPECT_EQ(record.get<float>(0), 10.0);
    EXPECT_EQ(record.get<int>(1), 20);
    EXPECT_EQ(record.get<long long>(2), 30ll);
}

TEST(RecordTest, NormalRecord_GetValues_InvalidValues) {
    Record record(str_vec{"  10 .5  ", " 20 1", "30 20", " aa ", "a4", "45ll"});
    EXPECT_EQ(record.get<float>(0), std::nullopt);
    EXPECT_EQ(record.get<int>(1), std::nullopt);
    EXPECT_EQ(record.get<long long>(2), std::nullopt);
    EXPECT_EQ(record.get<int>(3), std::nullopt);
    EXPECT_EQ(record.get<long>(4), std::nullopt);
    EXPECT_EQ(record.get<unsigned int>(5), std::nullopt);
}

TEST(RecordTest, NormalRecord_GetValues_InvalidValuesAsArithmeticsButValidAsString) {
    Record record(str_vec{"  10 .5  ", " 20 1", "30 20", " aa ", "a4", "45ll"});
    EXPECT_EQ(record.get<std::string_view>(0), "  10 .5  ");
    EXPECT_EQ(record.get<std::string_view>(1), " 20 1");
    EXPECT_EQ(record.get<std::string>(2), "30 20");
    EXPECT_EQ(record.get<std::string_view>(3), " aa ");
    EXPECT_EQ(record.get<std::string_view>(4), "a4");
    EXPECT_EQ(record.get<std::string>(5), "45ll");
}

TEST(RecordTest, NormalRecord_GetValues_ColumnNameAccess) {
    Record record(
        str_vec{"Bożydar", "21", "2456.78"},
        str_vec{"name", "age", "salary"}
    );
    EXPECT_EQ(record.get("name"), "Bożydar");
    EXPECT_EQ(record.get<int>("age"), 21);
    EXPECT_EQ(record.get<double>("salary"), 2456.78);
}

TEST(RecordTest, NormalRecord_GetValues_NulloptOnWrongColumnName) {
    Record record(
        str_vec{"Bożydar", "21", "2456.78"},
        str_vec{"name", "age", "salary"}
    );
    EXPECT_EQ(record.get(""), std::nullopt);
    EXPECT_EQ(record.get<int>("ages"), std::nullopt);
    EXPECT_EQ(record.get("country"), std::nullopt);
}

TEST(RecordTest, NormalRecord_SubscriptOperator_ValidAsString) {
    Record record(str_vec{"  10 .5  ", " 20 1", "30 20", " aa ", "a4", "45ll"});
    EXPECT_EQ(record[0], "  10 .5  ");
    EXPECT_EQ(record[1], " 20 1");
    EXPECT_EQ(record[2], "30 20");
    EXPECT_EQ(record[3], " aa ");
    EXPECT_EQ(record[4], "a4");
    EXPECT_EQ(record[5], "45ll");
}

TEST(RecordTest, NormalRecord_SubscriptOperator_ThrowOnWrongIndex) {
    Record record(str_vec{"  10 .5  ", " 20 1", "30 20", " aa ", "a4", "45ll"});
    EXPECT_THROW(record[10], std::out_of_range);
    EXPECT_THROW(record[100], std::out_of_range);
    EXPECT_THROW(record[21], std::out_of_range);
    EXPECT_THROW(record[13], std::out_of_range);
    EXPECT_THROW(record[42], std::out_of_range);
    EXPECT_THROW(record[6], std::out_of_range);
}

TEST(RecordTest, NormalRecord_SubscriptOperator_ColumnNameAccess) {
    Record record(
        str_vec{"Bożydar", "21", "Poland"},
        str_vec{"name", "age", "country"}
    );
    EXPECT_EQ(record[0], record["name"]);
    EXPECT_EQ(record["name"], "Bożydar");
    EXPECT_EQ(record[1], record["age"]);
    EXPECT_EQ(record["age"], "21");
    EXPECT_EQ(record[2], record["country"]);
    EXPECT_EQ(record["country"], "Poland");
}

TEST(RecordTest, NormalRecord_SubscriptOperator_ThrowOnWrongColumnName) {
    Record record(
        str_vec{"Bożydar", "21", "Poland"},
        str_vec{"name", "age", "country"}
    );
    EXPECT_THROW(record[""], RecordColumnNameError);
    EXPECT_THROW(record["ages"], RecordColumnNameError);
    EXPECT_THROW(record["counters"], RecordColumnNameError);
}