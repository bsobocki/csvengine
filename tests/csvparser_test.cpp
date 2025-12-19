#include <gtest/gtest.h>

#include <csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

class ParserTest : public ::testing::Test {
protected:
    Parser strict_parser{{.parse_mode = Config::ParseMode::strict}};
    Parser lenient_parser{{.parse_mode = Config::ParseMode::lenient}};
    Parser no_quote_parser{{.has_quoting = false}};

    void ExpectParse(Parser& parser,
                std::string_view input,
                Parser::ParseStatus expected_status,
                const std::vector<std::string_view>& expected_fields = {}){
        EXPECT_EQ(parser.parse(input), expected_status);
        if (!expected_fields.empty()) {
            EXPECT_EQ(parser.peek_fields(), expected_fields);
        }
    }
};

// TODO: add tests that check if quotes working correctly -- you cannot use quotes literals without quoting enabled for the field!!
// """"normal"""" -- doesn't allowed -- it needs to be wrapped in quoting " at the start and at the end like in the example below 

TEST_F(ParserTest, MoveFields_EmptyFields) {
    std::vector<std::string> expected_fields = {"Mark", "is", "quite","\"normal\""};
    std::vector<std::string_view> expected_fields_view = std::vector<std::string_view>(expected_fields.begin(), expected_fields.end());
    ExpectParse(strict_parser, "\"Mark\",is,quite,\"\"\"normal\"\"\"\n", Parser::ParseStatus::complete, expected_fields_view);
    EXPECT_EQ(strict_parser.move_fields(),  expected_fields);
    EXPECT_TRUE(strict_parser.peek_fields().empty());
    EXPECT_TRUE(strict_parser.move_fields().empty());
}

TEST_F(ParserTest, MoveFields_MoveData) {
    EXPECT_TRUE(strict_parser.move_fields().empty());
    EXPECT_TRUE(strict_parser.peek_fields().empty());
}

TEST_F(ParserTest, StrictParsing_Malformed_QuoteInUnquotedField) {
    ExpectParse(strict_parser,  R"(aa"ada","normal")", Parser::ParseStatus::fail);
}

TEST_F(ParserTest, StrictParsing_Malformed_ContentAfterClosingQuote) {
    ExpectParse(strict_parser,  R"("something""different"here,next)", Parser::ParseStatus::fail);
}

TEST_F(ParserTest, StrictParsing_CorrectQuoting_NoContentAfterClosingQuote) {
    ExpectParse(strict_parser,
        "\"something\"\"different\",next\n",
        Parser::ParseStatus::complete,
        {"something\"different", "next"});
}

TEST_F(ParserTest, StrictParsing_CorrectQuoting_NeedMoreDataWithLastCharAsQuote) {
    ExpectParse(strict_parser,  "\"something\"", Parser::ParseStatus::need_more_data);
    ExpectParse(strict_parser,  "\"different\"", Parser::ParseStatus::need_more_data);
    ExpectParse(strict_parser,  ",next\n", Parser::ParseStatus::complete, {"something\"different", "next"});
}

TEST_F(ParserTest, StrictParsing_NewlineAndDelimiterInQuotes) {
    ExpectParse(strict_parser,  "\"something", Parser::ParseStatus::need_more_data, {"something"});
    ExpectParse(strict_parser, "\n,\",different,\"", Parser::ParseStatus::need_more_data, {"something\n,","different", ""});
    ExpectParse(strict_parser, ",next\"\n", Parser::ParseStatus::complete, {"something\n,","different", ",next"});

    std::vector<std::string> expected_fields = {"something\n,","different", ",next"};
    EXPECT_EQ(strict_parser.move_fields(), expected_fields);
}

TEST_F(ParserTest, CustomDelimiter_Semicolon) {
    Parser semi_parser{{.delimiter = ';'}};
    ExpectParse(semi_parser, "a;b;c\n", 
                Parser::ParseStatus::complete, {"a", "b", "c"});
}