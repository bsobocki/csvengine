#include <gtest/gtest.h>

#include <csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

TEST(ParserTest, StrictParsing_Malformed_QuoteInUnquotedField) {
    std::string input = R"(aa"ada","normal")";
    std::string_view data = input;
    Parser parser({.parse_mode = Config::ParseMode::strict});
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::fail);
}

TEST(ParserTest, StrictParsing_Malformed_ContentAfterClosingQuote) {
    std::string input = R"("something""different"here,next)";
    std::string_view data = input;
    Parser parser({.parse_mode = Config::ParseMode::strict});
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::fail);
}

TEST(ParserTest, StrictParsing_CorrectQuoting_NoContentAfterClosingQuote) {
    std::string input = "\"something\"\"different\",next\n";
    std::string_view data = input;
    Parser parser({.parse_mode = Config::ParseMode::strict});
    std::vector<std::string> expectedFields = {"something\"different", "next"};
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::complete);
    EXPECT_EQ(parser.move_fields(), expectedFields);
}

TEST(ParserTest, StrictParsing_CorrectQuoting_NeedMoreDataWithLastCharAsQuote) {
    Parser parser({.parse_mode = Config::ParseMode::strict});
    
    std::string input = "\"something\"";
    std::string_view data = input;
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::need_more_data);

    input = "\"different\"";
    data = input;
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::need_more_data);
    
    input = ",next\n";
    data = input;
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::complete);

    std::vector<std::string> expectedFields = {"something\"different", "next"};
    EXPECT_EQ(parser.move_fields(), expectedFields);
}

TEST(ParserTest, StrictParsing_NewlineAndDelimiterInQuotes) {
    Parser parser({.parse_mode = Config::ParseMode::strict});
    
    std::string input = "\"something";
    std::string_view data = input;
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::need_more_data);

    input = "\n,\",different,\"";
    data = input;
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::need_more_data);
    
    input = ",next\"\n";
    data = input;
    EXPECT_EQ(parser.parse(data), Parser::ParseStatus::complete);

    std::vector<std::string> expectedFields = {"something\n,","different", ",next"};
    EXPECT_EQ(parser.move_fields(), expectedFields);
}