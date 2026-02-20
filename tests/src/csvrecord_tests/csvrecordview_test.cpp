#include <csvrecord/csvrecord.hpp>
#include <gtest/gtest.h>

using namespace csv;

using str_view_vec = std::vector<std::string_view>;
using str_vec = std::vector<std::string>;

TEST(RecordViewTest, CreateEmptyRecordView_NoThrow) {
    EXPECT_NO_THROW(RecordView RecordView);
}

TEST(RecordViewTest, EmptyRecordView_NulloptOnGet) {
    RecordView RecordView;
    EXPECT_EQ(RecordView.get<int>(0), std::nullopt);
    EXPECT_EQ(RecordView.get<int>(10), std::nullopt);
    EXPECT_EQ(RecordView.get<int>("name"), std::nullopt);
    EXPECT_EQ(RecordView.get<int>(""), std::nullopt);
}

TEST(RecordViewTest, NormalRecordView_GetIntValues) {
    RecordView RecordView(str_view_vec{"10", "20", "30"});
    EXPECT_EQ(RecordView.get<int>(0), 10);
    EXPECT_EQ(RecordView.get<int>(1), 20);
    EXPECT_EQ(RecordView.get<int>(2), 30);
}

TEST(RecordViewTest, NormalRecordView_GetStringValues) {
    RecordView RecordView(str_view_vec{"10", "20", "30", "mamma mia!", "   alehandro!  "});
    EXPECT_EQ(RecordView.get<std::string>(0), "10");
    EXPECT_EQ(RecordView.get<std::string>(1), "20");
    EXPECT_EQ(RecordView.get<std::string>(2), "30");
    EXPECT_EQ(RecordView.get<std::string>(3), "mamma mia!");
    EXPECT_EQ(RecordView.get<std::string>(4), "   alehandro!  ");
}

TEST(RecordViewTest, NormalRecordView_GetDoubleValues) {
    RecordView RecordView(str_view_vec{"10", "2.50", "3.506"});
    EXPECT_EQ(RecordView.get<double>(0), 10.0);
    EXPECT_EQ(RecordView.get<double>(1), 2.5);
    EXPECT_EQ(RecordView.get<double>(2), 3.506);
}

TEST(RecordViewTest, NormalRecordView_NulloptOnGetWithInvalidIndex) {
    RecordView RecordView(str_view_vec{"10", "20", "30"});
    EXPECT_EQ(RecordView.get<int>(4), std::nullopt);
    EXPECT_EQ(RecordView.get<int>(10), std::nullopt);
}

TEST(RecordViewTest, NormalRecordView_GetValues_TrailingSpacesRemovedForArithmeticsValues) {
    RecordView RecordView(str_view_vec{"  10   ", " 20 ", "   30      "});
    EXPECT_EQ(RecordView.get<float>(0), 10.0);
    EXPECT_EQ(RecordView.get<int>(1), 20);
    EXPECT_EQ(RecordView.get<long long>(2), 30ll);
}

TEST(RecordViewTest, NormalRecordView_GetValues_InvalidValues) {
    RecordView RecordView(str_view_vec{"  10 .5  ", " 20 1", "30 20", " aa ", "a4", "45ll"});
    EXPECT_EQ(RecordView.get<float>(0), std::nullopt);
    EXPECT_EQ(RecordView.get<int>(1), std::nullopt);
    EXPECT_EQ(RecordView.get<long long>(2), std::nullopt);
    EXPECT_EQ(RecordView.get<int>(3), std::nullopt);
    EXPECT_EQ(RecordView.get<long>(4), std::nullopt);
    EXPECT_EQ(RecordView.get<unsigned int>(5), std::nullopt);
}

TEST(RecordViewTest, NormalRecordView_GetValues_InvalidValuesAsArithmeticsButValidAsString) {
    RecordView RecordView(str_view_vec{"  10 .5  ", " 20 1", "30 20", " aa ", "a4", "45ll"});
    EXPECT_EQ(RecordView.get<std::string_view>(0), "  10 .5  ");
    EXPECT_EQ(RecordView.get<std::string_view>(1), " 20 1");
    EXPECT_EQ(RecordView.get<std::string>(2), "30 20");
    EXPECT_EQ(RecordView.get<std::string_view>(3), " aa ");
    EXPECT_EQ(RecordView.get<std::string_view>(4), "a4");
    EXPECT_EQ(RecordView.get<std::string>(5), "45ll");
}

TEST(RecordViewTest, NormalRecordView_GetValues_ColumnNameAccess) {
    RecordView RecordView(
        str_view_vec{"Bożydar", "21", "2456.78"},
        str_vec{"name", "age", "salary"}
    );
    EXPECT_EQ(RecordView.get("name"), "Bożydar");
    EXPECT_EQ(RecordView.get<int>("age"), 21);
    EXPECT_EQ(RecordView.get<double>("salary"), 2456.78);
}

TEST(RecordViewTest, NormalRecordView_GetValues_NulloptOnWrongColumnName) {
    RecordView RecordView(
        str_view_vec{"Bożydar", "21", "2456.78"},
        str_vec{"name", "age", "salary"}
    );
    EXPECT_EQ(RecordView.get(""), std::nullopt);
    EXPECT_EQ(RecordView.get<int>("ages"), std::nullopt);
    EXPECT_EQ(RecordView.get("country"), std::nullopt);
}

TEST(RecordViewTest, NormalRecordView_SubscriptOperator_ValidAsString) {
    RecordView RecordView(str_view_vec{"  10 .5  ", " 20 1", "30 20", " aa ", "a4", "45ll"});
    EXPECT_EQ(RecordView[0], "  10 .5  ");
    EXPECT_EQ(RecordView[1], " 20 1");
    EXPECT_EQ(RecordView[2], "30 20");
    EXPECT_EQ(RecordView[3], " aa ");
    EXPECT_EQ(RecordView[4], "a4");
    EXPECT_EQ(RecordView[5], "45ll");
}

TEST(RecordViewTest, NormalRecordView_SubscriptOperator_ColumnNameAccess) {
    RecordView RecordView(
        str_view_vec{"Bożydar", "21", "Poland"},
        str_vec{"name", "age", "country"}
    );
    EXPECT_EQ(RecordView[0], RecordView["name"]);
    EXPECT_EQ(RecordView["name"], "Bożydar");
    EXPECT_EQ(RecordView[1], RecordView["age"]);
    EXPECT_EQ(RecordView["age"], "21");
    EXPECT_EQ(RecordView[2], RecordView["country"]);
    EXPECT_EQ(RecordView["country"], "Poland");
}

TEST(RecordViewTest, NormalRecordView_SubscriptOperator_ThrowOnWrongColumnName) {
    RecordView RecordView(
        str_view_vec{"Bożydar", "21", "Poland"},
        str_vec{"name", "age", "country"}
    );
    EXPECT_THROW(RecordView[""], RecordColumnNameError);
    EXPECT_THROW(RecordView["ages"], RecordColumnNameError);
    EXPECT_THROW(RecordView["counters"], RecordColumnNameError);
}