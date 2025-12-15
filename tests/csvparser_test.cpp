#include <gtest/gtest.h>

#include <csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

TEST(ParserTest, StrictParsing_Malformed_QuoteInUnquotedField) {
    std::string input = R"(aa"ada","normal")";
    Parser parser();
    EXPECT_EQ(parser.parse(input), Parser::ParseStatus::fail);
}

TEST(ParserTest, StrictParsing_Malformed_ContentAfterClosingQuote) {
    std::string input = R"("something""different"here,next)";
    Parser parser();
    EXPECT_EQ(parser.parse(input), Parser::ParseStatus::fail);
}

TEST(ParserTest, StrictParsing_CorrectQuoting_NoContentAfterClosingQuote) {
    std::string input = R"("something""different",next)";
    Parser parser();
    std::vector<std::string> expectedFields = {"something""different", "next"};
    EXPECT_EQ(parser.parse(input), Parser::ParseStatus::complete);
    EXPECT_EQ(parser.fields(), expectedFields);
}