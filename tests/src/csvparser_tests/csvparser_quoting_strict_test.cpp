#include <gtest/gtest.h>

#include <csvparser/csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

class StrictParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser<std::string>> strict_parser = make_parser({.parse_mode = Config::ParseMode::strict});
    std::unique_ptr<Parser<std::string>> strict_parser_crlf = make_parser({
        .parse_mode = Config::ParseMode::strict,
        .line_ending = Config::LineEnding::crlf
    });
    std::unique_ptr<Parser<std::string>> strict_parser_cr = make_parser({
        .parse_mode = Config::ParseMode::strict,
        .line_ending = Config::LineEnding::cr
    });
    std::unique_ptr<Parser<std::string>> strict_parser_lf = make_parser({
        .parse_mode = Config::ParseMode::strict,
        .line_ending = Config::LineEnding::lf
    });
    std::unique_ptr<Parser<std::string>> semi_parser = make_parser({.delimiter = ';'});

    void ExpectParse(std::unique_ptr<Parser<std::string>>& parser,
                std::string_view input,
                ParseStatus expected_status,
                const std::vector<std::string>& expected_fields = {}){
        EXPECT_EQ(parser->parse(input), expected_status);
        if (!expected_fields.empty()) {
            EXPECT_EQ(parser->fields(), expected_fields);
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
    EXPECT_EQ(strict_parser->fields(), std::vector<std::string>{"hello world"});
}

TEST_F(StrictParserTest, Buffer_IncompleteQuotedField) {
    EXPECT_EQ(strict_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("lo\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), std::vector<std::string>{"hello"});
}

TEST_F(StrictParserTest, Buffer_QuoteAtBufferEnd_FollowedByNewline) {
    EXPECT_EQ(strict_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), std::vector<std::string>{"hello"});
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
    EXPECT_EQ(strict_parser->fields(), expected_fields);
}

TEST_F(StrictParserTest, Buffer_SplitEscapedQuote) {
    EXPECT_EQ(strict_parser->parse("\"a\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\"b\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), std::vector<std::string>{"a\"b"});
}

TEST_F(StrictParserTest, Buffer_EmptyBuffer) {
    EXPECT_EQ(strict_parser->parse(""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->consumed(), 0);
}

TEST_F(StrictParserTest, Buffer_MultipleChunks) {
    EXPECT_EQ(strict_parser->parse("a,"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("b,"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("c\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), (std::vector<std::string>{"a", "b", "c"}));
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
    EXPECT_EQ(strict_parser->fields(), (std::vector<std::string>{"a", "b"}));
}

// ============================================================
// CUSTOM DELIMITER
// ============================================================

TEST_F(StrictParserTest, CustomDelimiter_Tab) {
    std::unique_ptr<Parser<std::string>> tab_parser = make_parser({.delimiter = '\t'});
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
    EXPECT_EQ(strict_parser->fields(), expected_fields);
    strict_parser->reset();
    EXPECT_EQ(strict_parser->fields(), std::vector<std::string>{});
}

TEST_F(StrictParserTest, Reset_ClearsState) {
    strict_parser->parse("\"hello");
    strict_parser->reset();

    EXPECT_EQ(strict_parser->parse("world\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), std::vector<std::string>{"world"});
}

TEST_F(StrictParserTest, Reset_ClearsPendingQuote) {
    strict_parser->parse("\"hello\"");
    strict_parser->reset();
    EXPECT_EQ(strict_parser->parse("world\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), std::vector<std::string>{"world"});
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


// ============================================================
// CRLF (strict) behavior
// ============================================================

TEST_F(StrictParserTest, CRLF_Accepts_CRLF_Strips_CR) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(StrictParserTest, CRLF_DoNotAccepts_LF_Only) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\n"), ParseStatus::fail);
}

TEST_F(StrictParserTest, CRLF_EmptyLine_DoesNotCrash_AndConsumesOneRecord) {
    EXPECT_EQ(strict_parser_crlf->parse("\r\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{""}));
    EXPECT_EQ(strict_parser_crlf->consumed(), 2u);
}

TEST_F(StrictParserTest, CRLF_EmptyLine_WithLFOnly_Crash) {
    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::fail);
}

TEST_F(StrictParserTest, CRLF_ConsumesTwoBytes_ForCRLF) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(strict_parser_crlf->consumed(), 5u);
}

TEST_F(StrictParserTest, CRLF_MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(strict_parser_crlf->consumed(), 5u);

    strict_parser_crlf->reset();
    EXPECT_EQ(strict_parser_crlf->parse("c,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"c","d"}));
    EXPECT_EQ(strict_parser_crlf->consumed(), 5u);
}

TEST_F(StrictParserTest, CRLF_PartialAcrossChunks_CRThenLF) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b\r"}));

    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b"}));
}

// ============================================================
// CR-only mode
// ============================================================

TEST_F(StrictParserTest, CR_Mode_Parses_CR_Terminated_Line) {
    EXPECT_EQ(strict_parser_cr->parse("a,b\rc,d\r"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_cr->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(strict_parser_cr->consumed(), 4u);
}

TEST_F(StrictParserTest, CR_Mode_DoesNotTreat_LF_AsTerminator) {
    EXPECT_EQ(strict_parser_cr->parse("a,b\n"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_cr->consumed(), 4u);
    EXPECT_EQ(strict_parser_cr->fields(), (std::vector<std::string>{"a","b\n"}));
}

// ============================================================
// LF-only mode
// ============================================================

TEST_F(StrictParserTest, LF_Mode_Treat_CR_AsData) {
    EXPECT_EQ(strict_parser_lf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_lf->consumed(), 4u);
    EXPECT_EQ(strict_parser_lf->fields(), (std::vector<std::string>{"a","b\r"}));
}

// ============================================================
// CRLF tests - strict behavior - \r cannot be treated as data
// ============================================================

TEST_F(StrictParserTest, Regression_NoNewlinePtr_Nullptr_IsHandled) {
    EXPECT_EQ(strict_parser_crlf->parse("abc"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->consumed(), 3u);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"abc"}));
}

TEST_F(StrictParserTest, CRLF_SplitAcrossChunks_CRThenLF_StripsCR) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a", "b\r"}));

    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a", "b"}));
}

TEST_F(StrictParserTest, CRLF_SplitAfterClosingQuote) {
    EXPECT_EQ(strict_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a\r"}));

    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a"}));
}

TEST_F(StrictParserTest, PendingCR_PopsWithoutChecks) {
    EXPECT_EQ(strict_parser_crlf->parse("a\r"), ParseStatus::need_more_data);
    strict_parser_crlf->reset();
    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::fail);
}

TEST_F(StrictParserTest, NeedMoreDataMustConsume) {
    EXPECT_EQ(strict_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->consumed(), 4);
}

TEST_F(StrictParserTest, CRLF_SplitAcrossBuffers_OutsideQuotes) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);

    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(StrictParserTest, CRLF_QuoteThenCRAtEnd_ProgressAndCorrectConsume) {
    EXPECT_EQ(strict_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->consumed(), 4u);
    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a"}));
}

TEST_F(StrictParserTest, SplitOutsideQuotes_CRThenLF_RemovesCR) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a", "b\r"}));
    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(StrictParserTest, SplitOutsideQuotes_CRThenChar_Fail) {
    EXPECT_EQ(strict_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a", "b\r"}));
    EXPECT_EQ(strict_parser_crlf->parse("a\r\n"), ParseStatus::fail);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a","b\r"}));
}

TEST_F(StrictParserTest, EmptyLineSplit_CRThenLF_ProducesEmptyRecord) {
    EXPECT_EQ(strict_parser_crlf->parse("\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{""}));
}

TEST_F(StrictParserTest, PendingCR_NotFollowedByLF_Fail) {
    EXPECT_EQ(strict_parser_crlf->parse("a\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->parse("x"), ParseStatus::fail);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a\r"}));
}

TEST_F(StrictParserTest, ClosingQuoteThenCRLF_InSameBuffer) {
    EXPECT_EQ(strict_parser_crlf->parse("\"a\"\r\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a"}));
}

TEST_F(StrictParserTest, ClosingQuoteThenCR_SplitThenLF_Completes) {
    EXPECT_EQ(strict_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a"}));
}

TEST_F(StrictParserTest, CRatTheEndOfBuffer_needMoreData_OnlyDataWithCRasData) {
    EXPECT_EQ(strict_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser_crlf->fields(), (std::vector<std::string>{"a\r"}));
}

// ============================================================
// Additional tests
// ============================================================

TEST_F(StrictParserTest, EOF_NoNewline_Unquoted_ReturnsNeedMoreData) {
    ExpectParse(strict_parser, "a,b,c", ParseStatus::need_more_data, {"a","b","c"});
}

TEST_F(StrictParserTest, EOF_NoNewline_QuotedClosed_ReturnsNeedMoreData) {
    ExpectParse(strict_parser, "\"a\",\"b\"", ParseStatus::need_more_data, {"a","b"});
}

TEST_F(StrictParserTest, TrailingDelimiter_EndOfBuffer_NeedMoreData_IncludesEmptyField) {
    ExpectParse(strict_parser, "a,", ParseStatus::need_more_data, {"a",""});
}

TEST_F(StrictParserTest, TrailingDelimiter_BeforeNewline_Completes_IncludesEmptyField) {
    ExpectParse(strict_parser, "a,\n", ParseStatus::complete, {"a",""});
}

TEST_F(StrictParserTest, Buffer_SplitEmptyQuotedField) {
    EXPECT_EQ(strict_parser->parse("\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), (std::vector<std::string>{""}));
}

TEST_F(StrictParserTest, Buffer_Split_EmptyQuotedField_ThenDelimiter) {
    EXPECT_EQ(strict_parser->parse("\"\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse(",x\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), (std::vector<std::string>{"", "x"}));
}

TEST_F(StrictParserTest, Buffer_Split_AfterDelimiter_BeforeOpenQuote) {
    EXPECT_EQ(strict_parser->parse("a,"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse("\"b\"\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(StrictParserTest, Buffer_ClosingQuoteAtEnd_FollowedByDelimiter) {
    EXPECT_EQ(strict_parser->parse("\"a\""), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->parse(",b\n"), ParseStatus::complete);
    EXPECT_EQ(strict_parser->fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(StrictParserTest, Strict_SpaceBeforeQuote_AfterDelimiter_Fails) {
    ExpectParse(strict_parser, "a, \"b\"\n", ParseStatus::fail);
}

TEST_F(StrictParserTest, Strict_GarbageAfterDelimiterBeforeQuote_Fails) {
    ExpectParse(strict_parser, "a,x\"b\"\n", ParseStatus::fail);
}

TEST_F(StrictParserTest, LF_MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    ExpectParse(strict_parser, "a,b\nc,d\n", ParseStatus::complete, {"a","b"});
    EXPECT_EQ(strict_parser->consumed(), 4u);
}

TEST_F(StrictParserTest, CRLF_CRInsideQuotes_IsData) {
    ExpectParse(strict_parser_crlf, "\"a\rb\"\r\n", ParseStatus::complete, {"a\rb"});
}

TEST_F(StrictParserTest, CRLF_QuotedFieldThenDelimiterThenCRLF) {
    ExpectParse(strict_parser_crlf, "\"a\",b\r\n", ParseStatus::complete, {"a","b"});
}

TEST_F(StrictParserTest, NeedMoreData_ConsumedCountsBytes_LF) {
    EXPECT_EQ(strict_parser->parse("ab"), ParseStatus::need_more_data);
    EXPECT_EQ(strict_parser->consumed(), 2u);
}

TEST_F(StrictParserTest, Quoted_FieldStartsWithDelimiterChar) {
    ExpectParse(strict_parser, "\",\"\n", ParseStatus::complete, {","});
}

TEST_F(StrictParserTest, Quoted_FieldStartsWithNewlineChar) {
    ExpectParse(strict_parser, "\"\n\"\n", ParseStatus::complete, {"\n"});
}