#include <gtest/gtest.h>

#include <csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

class SimpleParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser> simple_parser = make_parser({.has_quoting = false});

    void ExpectParse(std::unique_ptr<Parser>& parser,
                std::string_view input,
                ParseStatus expected_status,
                const std::vector<std::string>& expected_fields = {}){
        EXPECT_EQ(parser->parse(input), expected_status);
        if (!expected_fields.empty()) {
            EXPECT_EQ(parser->peek_fields(), expected_fields);
        }
    }
};

// ============================================================
// HAPPY PATH
// ============================================================

TEST_F(SimpleParserTest, Basic_EmptyFields) {
    ExpectParse(simple_parser, "a,,c\n", 
        ParseStatus::complete, {"a", "", "c"});
}

TEST_F(SimpleParserTest, Basic_AllEmptyFields) {
    ExpectParse(simple_parser, ",,\n", 
        ParseStatus::complete, {"", "", ""});
}

TEST_F(SimpleParserTest, Basic_SingleEmptyField) {
    ExpectParse(simple_parser, "\n", 
        ParseStatus::complete, {});
}

TEST_F(SimpleParserTest, NoQuoting_QuotesAreLiteral) {
    ExpectParse(simple_parser, "\"hello\"\n", 
                ParseStatus::complete, {"\"hello\""});
}

TEST_F(SimpleParserTest, NoQuoting_QuoteInMiddle) {
    ExpectParse(simple_parser, "hel\"lo\n", 
                ParseStatus::complete, {"hel\"lo"});
}

// ============================================================
// PARTIAL PARSING
// ============================================================

TEST_F(SimpleParserTest, Buffer_IncompleteUnquotedField) {
    EXPECT_EQ(simple_parser->parse("hello"), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse(" world\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->move_fields(), std::vector<std::string>{"hello world"});
}

TEST_F(SimpleParserTest, Buffer_IncompleteQuotedField) {
    EXPECT_EQ(simple_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse("lo\"\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->move_fields(), std::vector<std::string>{"\"hello\""});
}

TEST_F(SimpleParserTest, Buffer_QuoteAtBufferEnd_FollowedByNewline) {
    EXPECT_EQ(simple_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->move_fields(), std::vector<std::string>{"\"hello\""});
}

TEST_F(SimpleParserTest, SimpleParsing_CorrectQuoting_NeedMoreDataWithLastCharAsQuote) {
    ExpectParse(simple_parser,  "\"something\"", ParseStatus::need_more_data, {"\"something\""});
    ExpectParse(simple_parser,  "\"different\"", ParseStatus::need_more_data, {"\"something\"\"different\""});
    ExpectParse(simple_parser,  ",next\n", ParseStatus::complete, {"\"something\"\"different\"", "next"});
}

TEST_F(SimpleParserTest, SimpleParsing_NewlineAndDelimiterInQuotes_ParserDoesntClearFieldsOnItOwn) {
    ExpectParse(simple_parser, "\"something", ParseStatus::need_more_data, {"\"something"});
    EXPECT_EQ(simple_parser->consumed(), 10);
    ExpectParse(simple_parser, "\n,\",different,\"", ParseStatus::complete, {"\"something"});
    EXPECT_EQ(simple_parser->consumed(), 1);
    ExpectParse(simple_parser, ",\",different,\",next\"\n", ParseStatus::complete, {"\"something", "", "\"", "different", "\"", "next\""});

    EXPECT_EQ(simple_parser->move_fields(), std::vector<std::string>({"\"something", "", "\"", "different", "\"", "next\""}));
}

TEST_F(SimpleParserTest, Buffer_SplitEscapedQuote) {
    EXPECT_EQ(simple_parser->parse("\"a\""), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse("\"b\"\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->move_fields(), std::vector<std::string>{"\"a\"\"b\""});
}

TEST_F(SimpleParserTest, Buffer_EmptyBuffer) {
    EXPECT_EQ(simple_parser->parse(""), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->consumed(), 0);
    EXPECT_EQ(simple_parser->move_fields(), (std::vector<std::string>{}));
}

TEST_F(SimpleParserTest, Buffer_MultipleChunks) {
    EXPECT_EQ(simple_parser->parse("a,"), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse("b,"), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse("c\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->move_fields(), (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(SimpleParserTest, Buffer_SingleCharChunks) {
    for (char c : std::string("a,b\n")) {
        std::string s(1, c);
        auto status = simple_parser->parse(s);
        if (c == '\n') {
            EXPECT_EQ(status, ParseStatus::complete);
        } else {
            EXPECT_EQ(status, ParseStatus::need_more_data);
        }
    }
    EXPECT_EQ(simple_parser->move_fields(), (std::vector<std::string>{"a", "b"}));
}