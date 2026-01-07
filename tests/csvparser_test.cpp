#include <gtest/gtest.h>

#include <csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

class ParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser> strict_parser = make_parser({.parse_mode = Config::ParseMode::strict});
    std::unique_ptr<Parser> no_quote_parser = make_parser({.has_quoting = false});
    std::unique_ptr<Parser> semi_parser = make_parser({.delimiter = ';'});

    void ExpectParse(std::unique_ptr<Parser>& parser,
                std::string_view input,
                ParseStatus expected_status,
                const std::vector<std::string_view>& expected_fields = {}){
        EXPECT_EQ(parser->parse(input), expected_status);
        if (!expected_fields.empty()) {
            EXPECT_EQ(parser->peek_fields(), expected_fields);
        }
    }
};

// ============================================================
// BASIC PARSING - HAPPY PATH
// ============================================================

TEST_F(ParserTest, Basic_SingleField) {
    ExpectParse(strict_parser, "hello\n", 
        ParseStatus::complete, {"hello"});
}

TEST_F(ParserTest, Basic_MultipleFields) {
    ExpectParse(strict_parser, "a,b,c\n", 
        ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(ParserTest, Basic_EmptyFields) {
    ExpectParse(strict_parser, "a,,c\n", 
        ParseStatus::complete, {"a", "", "c"});
}

TEST_F(ParserTest, Basic_AllEmptyFields) {
    ExpectParse(strict_parser, ",,\n", 
        ParseStatus::complete, {"", "", ""});
}

TEST_F(ParserTest, Basic_SingleEmptyField) {
    ExpectParse(strict_parser, "\n", 
        ParseStatus::complete, {""});
}

// ============================================================
// MOVE FIELDS
// ============================================================

TEST_F(ParserTest, MoveFields_MoveData) {
    std::vector<std::string> expected_fields = {"Mark", "is", "quite","\"normal\""};
    std::vector<std::string_view> expected_fields_view = std::vector<std::string_view>(expected_fields.begin(), expected_fields.end());

    ExpectParse(strict_parser, "\"Mark\",is,quite,\"\"\"normal\"\"\"\n", ParseStatus::complete, expected_fields_view);
    EXPECT_EQ(strict_parser->move_fields(),  expected_fields);
    EXPECT_TRUE(strict_parser->peek_fields().empty());
    EXPECT_TRUE(strict_parser->move_fields().empty());
}

TEST_F(ParserTest, MoveFields_EmptyFields) {
    EXPECT_TRUE(strict_parser->move_fields().empty());
    EXPECT_TRUE(strict_parser->peek_fields().empty());
}
                    
                    
// ============================================================
// QUOTING
// ============================================================
                    
TEST_F(ParserTest, StrictParsing_Malformed_QuoteInUnquotedField) {
    ExpectParse(strict_parser,  R"(aa"ada","normal")", ParseStatus::fail);
}

TEST_F(ParserTest, StrictParsing_Malformed_ContentAfterClosingQuote) {
    ExpectParse(strict_parser,  R"("something""different"here,next)", ParseStatus::fail);
}

TEST_F(ParserTest, StrictParsing_CorrectQuoting_NoContentAfterClosingQuote) {
    ExpectParse(strict_parser,
        "\"something\"\"different\",next\n",
        ParseStatus::complete,
        {"something\"different", "next"});
}

TEST_F(ParserTest, Quoted_SimpleField) {
    ExpectParse(strict_parser, "\"hello\"\n", 
                ParseStatus::complete, {"hello"});
}

TEST_F(ParserTest, Quoted_FieldWithComma) {
    ExpectParse(strict_parser, "\"hello,world\"\n", 
                ParseStatus::complete, {"hello,world"});
}

TEST_F(ParserTest, Quoted_FieldWithNewline) {
    ExpectParse(strict_parser, "\"hello\nworld\"\n", 
                ParseStatus::complete, {"hello\nworld"});
}

TEST_F(ParserTest, Quoted_EscapedQuote) {
    ExpectParse(strict_parser, "\"hello\"\"world\"\n", 
                ParseStatus::complete, {"hello\"world"});
}

TEST_F(ParserTest, Quoted_OnlyEscapedQuote) {
    ExpectParse(strict_parser, "\"\"\"\"\n", 
                ParseStatus::complete, {"\""});
}

TEST_F(ParserTest, Quoted_MultipleEscapedQuotes) {
    ExpectParse(strict_parser, "\"\"\"\"\"\"\n", 
                ParseStatus::complete, {"\"\""});
}

TEST_F(ParserTest, Quoted_EmptyQuotedField) {
    ExpectParse(strict_parser, "\"\"\n", 
                ParseStatus::complete, {""});
}

TEST_F(ParserTest, Quoted_MixedQuotedAndUnquoted) {
    ExpectParse(strict_parser, "a,\"b,c\",d\n", 
                ParseStatus::complete, {"a", "b,c", "d"});
}

TEST_F(ParserTest, Quoted_QuotedFieldAtStart) {
    ExpectParse(strict_parser, "\"a\",b,c\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(ParserTest, Quoted_QuotedFieldAtEnd) {
    ExpectParse(strict_parser, "a,b,\"c\"\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(ParserTest, Quoted_LiteralQuotesWithoutQuoting_Fail) {
    ExpectParse(strict_parser, "\"Mark\",is,quite,\"\"normal\"\"\n", ParseStatus::fail);
}

TEST_F(ParserTest, Quoted_WrongQuoting_Fail) {
    ExpectParse(strict_parser, "\"Mark\",is,quite,\"\"\"\"normal\"\"\"\"\n", ParseStatus::fail);
}

TEST_F(ParserTest, NoQuoting_QuotesAreLiteral) {
    ExpectParse(no_quote_parser, "\"hello\"\n", 
                ParseStatus::complete, {"\"hello\""});
}

TEST_F(ParserTest, NoQuoting_QuoteInMiddle) {
    ExpectParse(no_quote_parser, "hel\"lo\n", 
                ParseStatus::complete, {"hel\"lo"});
}

// ============================================================
// PARTIAL PARSING
// ============================================================

TEST_F(ParserTest, Buffer_IncompleteUnquotedField) {
    EXPECT_EQ(strict_parser->parse("hello"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse(" world\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"hello world"});
}

TEST_F(ParserTest, Buffer_IncompleteQuotedField) {
    EXPECT_EQ(strict_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("lo\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"hello"});
}

TEST_F(ParserTest, Buffer_QuoteAtBufferEnd_FollowedByNewline) {
    EXPECT_EQ(strict_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"hello"});
}

TEST_F(ParserTest, StrictParsing_CorrectQuoting_NeedMoreDataWithLastCharAsQuote) {
    ExpectParse(strict_parser,  "\"something\"", ParseStatus::need_more_data);
    ExpectParse(strict_parser,  "\"different\"", ParseStatus::need_more_data);
    ExpectParse(strict_parser,  ",next\n", ParseStatus::complete, {"something\"different", "next"});
}

TEST_F(ParserTest, StrictParsing_NewlineAndDelimiterInQuotes) {
    ExpectParse(strict_parser,  "\"something", ParseStatus::need_more_data, {"something"});
    ExpectParse(strict_parser, "\n,\",different,\"", ParseStatus::need_more_data, {"something\n,","different", ""});
    ExpectParse(strict_parser, ",next\"\n", ParseStatus::complete, {"something\n,","different", ",next"});

    std::vector<std::string> expected_fields = {"something\n,","different", ",next"};
    EXPECT_EQ(strict_parser->move_fields(), expected_fields);
}

TEST_F(ParserTest, Buffer_SplitEscapedQuote) {
    EXPECT_EQ(strict_parser->parse("\"a\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\"b\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"a\"b"});
}

TEST_F(ParserTest, Buffer_EmptyBuffer) {
    EXPECT_EQ(strict_parser->parse(""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->consumed(), 0);
}

TEST_F(ParserTest, Buffer_MultipleChunks) {
    EXPECT_EQ(strict_parser->parse("a,"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("b,"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("c\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(ParserTest, Buffer_SingleCharChunks) {
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

TEST_F(ParserTest, CustomDelimiter_Tab) {
    std::unique_ptr<Parser> tab_parser = make_parser({.delimiter = '\t'});
    ExpectParse(tab_parser, "a\tb\tc\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(ParserTest, CustomDelimiter_Semicolon) {
    ExpectParse(semi_parser, "a;b;c\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(ParserTest, CustomDelimiter_CommaInFieldWithSemicolonDelim) {
    ExpectParse(semi_parser, "a,b;c,d\n", 
                ParseStatus::complete, {"a,b", "c,d"});
}

// ============================================================
// MALFORMED INPUT - STRICT MODE
// ============================================================

TEST_F(ParserTest, Strict_QuoteInMiddleOfUnquotedField) {
    ExpectParse(strict_parser, "hel\"lo\n", ParseStatus::fail);
}

TEST_F(ParserTest, Strict_ContentAfterClosingQuote) {
    ExpectParse(strict_parser, "\"hello\"world\n", ParseStatus::fail);
}

TEST_F(ParserTest, Strict_UnclosedQuote_AtEndOfInput) {
    EXPECT_EQ(strict_parser->parse("\"hello"), ParseStatus::need_more_data);
}

TEST_F(ParserTest, Strict_QuoteAfterContent) {
    ExpectParse(strict_parser, "hello\",world\n", ParseStatus::fail);
}

TEST_F(ParserTest, Strict_SpaceBeforeQuote) {
    ExpectParse(strict_parser, " \"hello\"\n", ParseStatus::fail);
}

TEST_F(ParserTest, Strict_SpaceAfterQuote) {
    ExpectParse(strict_parser, "\"hello\" \n", ParseStatus::fail);
}

// ============================================================
// RESET FUNCTIONALITY
// ============================================================

TEST_F(ParserTest, Reset_ClearsFields) {
    std::vector<std::string_view> expected_fields = {"a", "b"};
    ExpectParse(strict_parser, "a,b\nabc", ParseStatus::complete, expected_fields);
    EXPECT_EQ(strict_parser->peek_fields(), expected_fields);
    strict_parser->reset();
    EXPECT_EQ(strict_parser->peek_fields(), std::vector<std::string_view>{});
}

TEST_F(ParserTest, Reset_ClearsState) {
    strict_parser->parse("\"hello");  // In quotes
    strict_parser->reset();
    // Should parse fresh, not continue quoted state
    EXPECT_EQ(strict_parser->parse("world\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"world"});
}

TEST_F(ParserTest, Reset_ClearsPendingQuote) {
    strict_parser->parse("\"hello\"");  // Pending quote
    strict_parser->reset();
    EXPECT_EQ(strict_parser->parse("world\n"), ParseStatus::complete); // fails without reset
    EXPECT_EQ(strict_parser->move_fields(), std::vector<std::string>{"world"}); // only data after reset
}

TEST_F(ParserTest, Reset_ClearsConsumed) {
    strict_parser->parse("hello\n");
    strict_parser->reset();
    EXPECT_EQ(strict_parser->consumed(), 0);
}

// ============================================================
// EDGE CASES
// ============================================================

TEST_F(ParserTest, Edge_OnlyNewline) {
    ExpectParse(strict_parser, "\n", ParseStatus::complete, {""});
}

TEST_F(ParserTest, Edge_OnlyDelimiter) {
    ExpectParse(strict_parser, ",", ParseStatus::need_more_data, {"", ""});
}

TEST_F(ParserTest, Edge_DelimiterThenNewline) {
    ExpectParse(strict_parser, ",\n", ParseStatus::complete, {"", ""});
}

TEST_F(ParserTest, Edge_ManyEmptyFields) {
    ExpectParse(strict_parser, ",,,,\n", ParseStatus::complete, {"", "", "", "", ""});
}

TEST_F(ParserTest, Edge_QuotedEmpty) {
    ExpectParse(strict_parser, "\"\",\"\"\n", ParseStatus::complete, {"", ""});
}

TEST_F(ParserTest, Edge_VeryLongField) {
    std::string long_field(10000, 'a');
    std::string input = long_field + "\n";
    ExpectParse(strict_parser, input, ParseStatus::complete, {long_field});
}

TEST_F(ParserTest, Edge_VeryLongQuotedField) {
    std::string long_field(10000, 'a');
    std::string input = "\"" + long_field + "\"\n";
    ExpectParse(strict_parser, input, ParseStatus::complete, {long_field});
}