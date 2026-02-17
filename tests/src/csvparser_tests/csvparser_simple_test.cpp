#include <gtest/gtest.h>

#include <csvparser/csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

class SimpleParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser<std::string>> simple_parser = make_parser({.has_quoting = false});

    void ExpectParse(std::unique_ptr<Parser<std::string>>& parser,
                std::string_view input,
                ParseStatus expected_status,
                std::optional<std::vector<std::string>> expected_fields = std::nullopt){
        EXPECT_EQ(parser->parse(input), expected_status);
        if (expected_fields != std::nullopt) {
            EXPECT_EQ(parser->fields(), *expected_fields);
        }
    }
};

// ============================================================
// HAPPY PATH
// ============================================================

TEST_F(SimpleParserTest, Basic_EmptyFields) {
    ExpectParse(simple_parser, "a,,c\n", 
        ParseStatus::complete, std::vector<std::string>{"a", "", "c"});
}

TEST_F(SimpleParserTest, Basic_AllEmptyFields) {
    ExpectParse(simple_parser, ",,\n", 
        ParseStatus::complete, std::vector<std::string>{"", "", ""});
}

TEST_F(SimpleParserTest, Basic_SingleEmptyField) {
    ExpectParse(simple_parser, "\n", 
        ParseStatus::complete, std::vector<std::string>{});
}

TEST_F(SimpleParserTest, NoQuoting_QuotesAreLiteral) {
    ExpectParse(simple_parser, "\"hello\"\n", 
                ParseStatus::complete, std::vector<std::string>{"\"hello\""});
}

TEST_F(SimpleParserTest, NoQuoting_QuoteInMiddle) {
    ExpectParse(simple_parser, "hel\"lo\n", 
                ParseStatus::complete, std::vector<std::string>{"hel\"lo"});
}

// ============================================================
// PARTIAL PARSING
// ============================================================

TEST_F(SimpleParserTest, Buffer_IncompleteUnquotedField) {
    EXPECT_EQ(simple_parser->parse("hello"), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse(" world\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->fields(), std::vector<std::string>{"hello world"});
}

TEST_F(SimpleParserTest, Buffer_IncompleteQuotedField) {
    EXPECT_EQ(simple_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse("lo\"\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->fields(), std::vector<std::string>{"\"hello\""});
}

TEST_F(SimpleParserTest, Buffer_QuoteAtBufferEnd_FollowedByNewline) {
    EXPECT_EQ(simple_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(simple_parser->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(simple_parser->fields(), std::vector<std::string>{"\"hello\""});
}

TEST_F(SimpleParserTest, SimpleParsing_CorrectQuoting_NeedMoreDataWithLastCharAsQuote) {
    ExpectParse(simple_parser,  "\"something\"",
        ParseStatus::need_more_data, std::vector<std::string>{"\"something\""});
    ExpectParse(simple_parser,  "\"different\"",
        ParseStatus::need_more_data, std::vector<std::string>{"\"something\"\"different\""});
    ExpectParse(simple_parser,  ",next\n",
        ParseStatus::complete, std::vector<std::string>{"\"something\"\"different\"", "next"});
}

TEST_F(SimpleParserTest, SimpleParsing_NewlineAndDelimiterInQuotes_ParserDoesntClearFieldsOnItOwn) {
    ExpectParse(simple_parser, "\"something",
        ParseStatus::need_more_data, std::vector<std::string>{"\"something"});

    EXPECT_EQ(simple_parser->consumed(), 10);

    ExpectParse(simple_parser, "\n,\",different,\"",
        ParseStatus::complete, std::vector<std::string>{"\"something"});
    
    EXPECT_EQ(simple_parser->consumed(), 1);

    ExpectParse(simple_parser, ",\",different,\",next\"\n",
        ParseStatus::complete, std::vector<std::string>{"\"something", "", "\"", "different", "\"", "next\""});

    EXPECT_EQ(simple_parser->fields(), std::vector<std::string>({"\"something", "", "\"", "different", "\"", "next\""}));
}

TEST_F(SimpleParserTest, Buffer_SplitEscapedQuote) {
    ExpectParse(simple_parser, "\"a\"", ParseStatus::need_more_data, std::vector<std::string>{"\"a\""});
    ExpectParse(simple_parser, "\"b\"\n", ParseStatus::complete, std::vector<std::string>{"\"a\"\"b\""});
    EXPECT_EQ(simple_parser->fields(), std::vector<std::string>{"\"a\"\"b\""});
}

TEST_F(SimpleParserTest, Buffer_EmptyBuffer) {
    ExpectParse(simple_parser, "", ParseStatus::need_more_data, (std::vector<std::string>{}));
    EXPECT_EQ(simple_parser->consumed(), 0);
    EXPECT_EQ(simple_parser->fields(), (std::vector<std::string>{}));
}

TEST_F(SimpleParserTest, Buffer_MultipleChunks) {
    ExpectParse(simple_parser, "a,", ParseStatus::need_more_data, (std::vector<std::string>{"a", ""}));
    ExpectParse(simple_parser, "b,", ParseStatus::need_more_data, (std::vector<std::string>{"a", "b", ""}));
    ExpectParse(simple_parser, "c\n", ParseStatus::complete, (std::vector<std::string>{"a", "b", "c"}));
    EXPECT_EQ(simple_parser->fields(), (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(SimpleParserTest, EmptyBufferThenData_DoesNotCrashAndParsesNormally) {
    ExpectParse(simple_parser, "", ParseStatus::need_more_data, (std::vector<std::string>{}));
    ExpectParse(simple_parser, "a\n", ParseStatus::complete, (std::vector<std::string>{"a"}));
    EXPECT_EQ(simple_parser->fields(), (std::vector<std::string>{"a"}));
}

TEST_F(SimpleParserTest, MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    ExpectParse(simple_parser, "a,b\nc,d\n", ParseStatus::complete, (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(simple_parser->consumed(), 4); // "a,b\n"

    simple_parser->reset();
    ExpectParse(simple_parser, "c,d\n", ParseStatus::complete, (std::vector<std::string>{"c","d"}));
}

TEST_F(SimpleParserTest, TrailingDelimiter) {
    ExpectParse(simple_parser, "a,b,\n", ParseStatus::complete, (std::vector<std::string>{"a","b",""}));
}

TEST_F(SimpleParserTest, DelimiterOnlyWithNewline) {
    ExpectParse(simple_parser, ",\n", ParseStatus::complete, (std::vector<std::string>{"",""}));
}

TEST_F(SimpleParserTest, OnlyDelimiter) {
    ExpectParse(simple_parser, ",", ParseStatus::need_more_data, (std::vector<std::string>{"",""}));
}

TEST_F(SimpleParserTest, EOF_NoNewline_LastRecordReturnedViaMoveFields) {
    ExpectParse(simple_parser, "a,b", ParseStatus::need_more_data,
                std::vector<std::string>{"a","b"});
    EXPECT_EQ(simple_parser->fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(SimpleParserTest, EmptyLine_RecordIsEmptyVector) {
    ExpectParse(simple_parser, "\na\n", ParseStatus::complete, std::vector<std::string>{});
    EXPECT_EQ(simple_parser->consumed(), 1);
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
    EXPECT_EQ(simple_parser->fields(), (std::vector<std::string>{"a", "b"}));
}


// ============================================================
// CRLF (lenient) behavior
// ============================================================

TEST(SimpleParserLineEndingTest, CRLF_Accepts_CRLF_Strips_CR) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("a,b\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
}

TEST(SimpleParserLineEndingTest, CRLF_Accepts_LF_Only_As_Well_Lenient) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("a,b\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
}

TEST(SimpleParserLineEndingTest, CRLF_EmptyLine_DoesNotCrash_AndConsumesOneRecord) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{}));
    EXPECT_EQ(p->consumed(), 2u);
}

TEST(SimpleParserLineEndingTest, CRLF_EmptyLine_WithLFOnly_DoesNotCrash) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{}));
    EXPECT_EQ(p->consumed(), 1u);
}

TEST(SimpleParserLineEndingTest, CRLF_ConsumesTwoBytes_ForCRLF) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(p->consumed(), 5u); // "a,b\r\n" = 5
}

TEST(SimpleParserLineEndingTest, CRLF_MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});

    EXPECT_EQ(p->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(p->consumed(), 5u);

    p->reset();
    EXPECT_EQ(p->parse("c,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"c","d"}));
    EXPECT_EQ(p->consumed(), 5u);
}

TEST(SimpleParserLineEndingTest, CRLF_PartialAcrossChunks_CRThenLF) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});

    EXPECT_EQ(p->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(p->parse("\n"), ParseStatus::complete);

    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
}

// ============================================================
// CR-only mode (if you support it)
// ============================================================

TEST(SimpleParserLineEndingTest, CR_Mode_Parses_CR_Terminated_Line) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::cr});
    EXPECT_EQ(p->parse("a,b\rc,d\r"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(p->consumed(), 4u);
}

TEST(SimpleParserLineEndingTest, CR_Mode_DoesNotTreat_LF_AsTerminator) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::cr});
    EXPECT_EQ(p->parse("a,b\n"), ParseStatus::need_more_data);
    EXPECT_EQ(p->consumed(), 4u);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b\n"}));
}

// ============================================================
// Regression: no UB when newline not found
// (this catches the "compute newline_pos before nullptr-check" bug)
// ============================================================

TEST(SimpleParserLineEndingTest, Regression_NoNewlinePtr_Nullptr_IsHandled) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("abc"), ParseStatus::need_more_data);
    EXPECT_EQ(p->consumed(), 3u);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"abc"}));
}

TEST(SimpleParserLineEndingTest, CRLF_SplitAcrossChunks_CRThenLF_StripsCR) {
    auto p = make_parser({.has_quoting = false, .line_ending = Config::LineEnding::crlf});

    EXPECT_EQ(p->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a", "b\r"}));

    EXPECT_EQ(p->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a", "b"}));
}
