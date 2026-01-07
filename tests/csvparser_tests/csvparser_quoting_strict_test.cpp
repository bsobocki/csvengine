#include <gtest/gtest.h>

#include <csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

class StrictParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser> strict_parser = make_parser({.parse_mode = Config::ParseMode::strict});
    std::unique_ptr<Parser> semi_parser = make_parser({.delimiter = ';'});

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
// BASIC PARSING - HAPPY PATH
// ============================================================

TEST_F(StrictParserTest, Basic_SingleField) {
    ExpectParse(strict_parser, "hello\n", 
        ParseStatus::complete, {"hello"});
}

TEST_F(StrictParserTest, Basic_MultipleFields) {
    ExpectParse(strict_parser, "a,b,c\n", 
        ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(StrictParserTest, Basic_EmptyFields) {
    ExpectParse(strict_parser, "a,,c\n", 
        ParseStatus::complete, {"a", "", "c"});
}

TEST_F(StrictParserTest, Basic_AllEmptyFields) {
    ExpectParse(strict_parser, ",,\n", 
        ParseStatus::complete, {"", "", ""});
}

TEST_F(StrictParserTest, Basic_SingleEmptyField) {
    ExpectParse(strict_parser, "\n", 
        ParseStatus::complete, {""});
}

// ============================================================
// MOVE FIELDS
// ============================================================

TEST_F(StrictParserTest, MoveFields_MoveData) {
    std::vector<std::string> expected_fields = {"Mark", "is", "quite","\"normal\""};

    ExpectParse(strict_parser, "\"Mark\",is,quite,\"\"\"normal\"\"\"\n", ParseStatus::complete, expected_fields);
    EXPECT_EQ(strict_parser->move_fields(),  expected_fields);
    EXPECT_TRUE(strict_parser->peek_fields().empty());
    EXPECT_TRUE(strict_parser->move_fields().empty());
}

TEST_F(StrictParserTest, MoveFields_EmptyFields) {
    EXPECT_TRUE(strict_parser->move_fields().empty());
    EXPECT_TRUE(strict_parser->peek_fields().empty());
}
                    
                    
// ============================================================
// QUOTING
// ============================================================
                    
TEST_F(StrictParserTest, StrictParsing_Malformed_QuoteInUnquotedField) {
    ExpectParse(strict_parser,  R"(aa"ada","normal")", ParseStatus::fail);
}

TEST_F(StrictParserTest, StrictParsing_Malformed_ContentAfterClosingQuote) {
    ExpectParse(strict_parser,  R"("something""different"here,next)", ParseStatus::fail);
}

TEST_F(StrictParserTest, StrictParsing_CorrectQuoting_NoContentAfterClosingQuote) {
    ExpectParse(strict_parser,
        "\"something\"\"different\",next\n",
        ParseStatus::complete,
        {"something\"different", "next"});
}

TEST_F(StrictParserTest, Quoted_SimpleField) {
    ExpectParse(strict_parser, "\"hello\"\n", 
                ParseStatus::complete, {"hello"});
}

TEST_F(StrictParserTest, Quoted_FieldWithComma) {
    ExpectParse(strict_parser, "\"hello,world\"\n", 
                ParseStatus::complete, {"hello,world"});
}

TEST_F(StrictParserTest, Quoted_FieldWithNewline) {
    ExpectParse(strict_parser, "\"hello\nworld\"\n", 
                ParseStatus::complete, {"hello\nworld"});
}

TEST_F(StrictParserTest, Quoted_EscapedQuote) {
    ExpectParse(strict_parser, "\"hello\"\"world\"\n", 
                ParseStatus::complete, {"hello\"world"});
}

TEST_F(StrictParserTest, Quoted_OnlyEscapedQuote) {
    ExpectParse(strict_parser, "\"\"\"\"\n", 
                ParseStatus::complete, {"\""});
}

TEST_F(StrictParserTest, Quoted_MultipleEscapedQuotes) {
    ExpectParse(strict_parser, "\"\"\"\"\"\"\n", 
                ParseStatus::complete, {"\"\""});
}

TEST_F(StrictParserTest, Quoted_EmptyQuotedField) {
    ExpectParse(strict_parser, "\"\"\n", 
                ParseStatus::complete, {""});
}

TEST_F(StrictParserTest, Quoted_MixedQuotedAndUnquoted) {
    ExpectParse(strict_parser, "a,\"b,c\",d\n", 
                ParseStatus::complete, {"a", "b,c", "d"});
}

TEST_F(StrictParserTest, Quoted_QuotedFieldAtStart) {
    ExpectParse(strict_parser, "\"a\",b,c\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(StrictParserTest, Quoted_QuotedFieldAtEnd) {
    ExpectParse(strict_parser, "a,b,\"c\"\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(StrictParserTest, Quoted_LiteralQuotesWithoutQuoting_Fail) {
    ExpectParse(strict_parser, "\"Mark\",is,quite,\"\"normal\"\"\n", ParseStatus::fail);
}

TEST_F(StrictParserTest, Quoted_WrongQuoting_Fail) {
    ExpectParse(strict_parser, "\"Mark\",is,quite,\"\"\"\"normal\"\"\"\"\n", ParseStatus::fail);
}

// ============================================================
// PARTIAL PARSING
// ============================================================

TEST_F(StrictParserTest, Buffer_IncompleteUnquotedField) {
    EXPECT_EQ(strict_parser->parse("hello"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse(" world\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"hello world"});
}

TEST_F(StrictParserTest, Buffer_IncompleteQuotedField) {
    EXPECT_EQ(strict_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("lo\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"hello"});
}

TEST_F(StrictParserTest, Buffer_QuoteAtBufferEnd_FollowedByNewline) {
    EXPECT_EQ(strict_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"hello"});
}

TEST_F(StrictParserTest, StrictParsing_CorrectQuoting_NeedMoreDataWithLastCharAsQuote) {
    ExpectParse(strict_parser,  "\"something\"", ParseStatus::need_more_data);
    ExpectParse(strict_parser,  "\"different\"", ParseStatus::need_more_data);
    ExpectParse(strict_parser,  ",next\n", ParseStatus::complete, {"something\"different", "next"});
}

TEST_F(StrictParserTest, StrictParsing_NewlineAndDelimiterInQuotes) {
    ExpectParse(strict_parser,  "\"something", ParseStatus::need_more_data, {"something"});
    ExpectParse(strict_parser, "\n,\",different,\"", ParseStatus::need_more_data, {"something\n,","different", ""});
    ExpectParse(strict_parser, ",next\"\n", ParseStatus::complete, {"something\n,","different", ",next"});

    std::vector<std::string> expected_fields = {"something\n,","different", ",next"};
    EXPECT_EQ(strict_parser->move_fields(), expected_fields);
}

TEST_F(StrictParserTest, Buffer_SplitEscapedQuote) {
    EXPECT_EQ(strict_parser->parse("\"a\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\"b\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"a\"b"});
}

TEST_F(StrictParserTest, Buffer_EmptyBuffer) {
    EXPECT_EQ(strict_parser->parse(""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->consumed(), 0);
}

TEST_F(StrictParserTest, Buffer_MultipleChunks) {
    EXPECT_EQ(strict_parser->parse("a,"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("b,"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("c\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(StrictParserTest, Buffer_SingleCharChunks) {
    for (char c : std::string("a,b\n")) {
        std::string s(1, c);
        auto status = strict_parser->parse(s);
        if (c == '\n') {
            EXPECT_EQ(status, ParseStatus::complete);
        } else {
            EXPECT_EQ(status, ParseStatus::need_more_data);
        }
    }
    EXPECT_EQ(strict_parser->move_fields(), (std::vector<std::string>{"a", "b"}));
}

// ============================================================
// CUSTOM DELIMITER
// ============================================================

TEST_F(StrictParserTest, CustomDelimiter_Tab) {
    std::unique_ptr<Parser> tab_parser = make_parser({.delimiter = '\t'});
    ExpectParse(tab_parser, "a\tb\tc\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(StrictParserTest, CustomDelimiter_Semicolon) {
    ExpectParse(semi_parser, "a;b;c\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(StrictParserTest, CustomDelimiter_CommaInFieldWithSemicolonDelim) {
    ExpectParse(semi_parser, "a,b;c,d\n", 
                ParseStatus::complete, {"a,b", "c,d"});
}

// ============================================================
// MALFORMED INPUT - STRICT MODE
// ============================================================

TEST_F(StrictParserTest, Strict_QuoteInMiddleOfUnquotedField) {
    ExpectParse(strict_parser, "hel\"lo\n", ParseStatus::fail);
}

TEST_F(StrictParserTest, Strict_ContentAfterClosingQuote) {
    ExpectParse(strict_parser, "\"hello\"world\n", ParseStatus::fail);
}

TEST_F(StrictParserTest, Strict_UnclosedQuote_AtEndOfInput) {
    EXPECT_EQ(strict_parser->parse("\"hello"), ParseStatus::need_more_data);
}

TEST_F(StrictParserTest, Strict_QuoteAfterContent) {
    ExpectParse(strict_parser, "hello\",world\n", ParseStatus::fail);
}

TEST_F(StrictParserTest, Strict_SpaceBeforeQuote) {
    ExpectParse(strict_parser, " \"hello\"\n", ParseStatus::fail);
}

TEST_F(StrictParserTest, Strict_SpaceAfterQuote) {
    ExpectParse(strict_parser, "\"hello\" \n", ParseStatus::fail);
}

// ============================================================
// RESET FUNCTIONALITY
// ============================================================

TEST_F(StrictParserTest, Reset_ClearsFields) {
    std::vector<std::string> expected_fields = {"a", "b"};
    ExpectParse(strict_parser, "a,b\nabc", ParseStatus::complete, expected_fields);
    EXPECT_EQ(strict_parser->peek_fields(), expected_fields);
    strict_parser->reset();
    EXPECT_EQ(strict_parser->peek_fields(), std::vector<std::string>{});
}

TEST_F(StrictParserTest, Reset_ClearsState) {
    strict_parser->parse("\"hello");  // In quotes
    strict_parser->reset();
    // Should parse fresh, not continue quoted state
    EXPECT_EQ(strict_parser->parse("world\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"world"});
}

TEST_F(StrictParserTest, Reset_ClearsPendingQuote) {
    strict_parser->parse("\"hello\"");  // Pending quote
    strict_parser->reset();
    EXPECT_EQ(strict_parser->parse("world\n"), ParseStatus::complete); // fails without reset
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"world"}); // only data after reset
}

TEST_F(StrictParserTest, Reset_ClearsConsumed) {
    strict_parser->parse("hello\n");
    strict_parser->reset();
    EXPECT_EQ(strict_parser->consumed(), 0);
}

// ============================================================
// EDGE CASES
// ============================================================

TEST_F(StrictParserTest, Edge_OnlyNewline) {
    ExpectParse(strict_parser, "\n", ParseStatus::complete, {""});
}

TEST_F(StrictParserTest, Edge_OnlyDelimiter) {
    ExpectParse(strict_parser, ",", ParseStatus::need_more_data, {"", ""});
}

TEST_F(StrictParserTest, Edge_DelimiterThenNewline) {
    ExpectParse(strict_parser, ",\n", ParseStatus::complete, {"", ""});
}

TEST_F(StrictParserTest, Edge_ManyEmptyFields) {
    ExpectParse(strict_parser, ",,,,\n", ParseStatus::complete, {"", "", "", "", ""});
}

TEST_F(StrictParserTest, Edge_QuotedEmpty) {
    ExpectParse(strict_parser, "\"\",\"\"\n", ParseStatus::complete, {"", ""});
}

TEST_F(StrictParserTest, Edge_VeryLongField) {
    std::string long_field(10000, 'a');
    std::string input = long_field + "\n";
    ExpectParse(strict_parser, input, ParseStatus::complete, {long_field});
}

TEST_F(StrictParserTest, Edge_VeryLongQuotedField) {
    std::string long_field(10000, 'a');
    std::string input = "\"" + long_field + "\"\n";
    ExpectParse(strict_parser, input, ParseStatus::complete, {long_field});
}