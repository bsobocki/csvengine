#include <gtest/gtest.h>

#include <csvparser/csvparser.hpp>
#include <csvconfig.hpp>
#include <testdata.hpp>

using namespace csv;

class SimdParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser<std::string>> simd_parser = make_parser({.has_quoting = false, .use_simd = true});

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

TEST_F(SimdParserTest, Basic_EmptyFields) {
    ExpectParse(simd_parser, "a,,c\n", 
        ParseStatus::complete, std::vector<std::string>{"a", "", "c"});
}

TEST_F(SimdParserTest, Basic_AllEmptyFields) {
    ExpectParse(simd_parser, ",,\n", 
        ParseStatus::complete, std::vector<std::string>{"", "", ""});
}

TEST_F(SimdParserTest, Basic_SingleEmptyField) {
    ExpectParse(simd_parser, "\n", 
        ParseStatus::complete, std::vector<std::string>{});
}

TEST_F(SimdParserTest, NoQuoting_QuotesAreLiteral) {
    ExpectParse(simd_parser, "\"hello\"\n", 
                ParseStatus::complete, std::vector<std::string>{"\"hello\""});
}

TEST_F(SimdParserTest, NoQuoting_QuoteInMiddle) {
    ExpectParse(simd_parser, "hel\"lo\n", 
                ParseStatus::complete, std::vector<std::string>{"hel\"lo"});
}

// ============================================================
// PARTIAL PARSING
// ============================================================

TEST_F(SimdParserTest, Buffer_IncompleteUnquotedField) {
    EXPECT_EQ(simd_parser->parse("hello"), ParseStatus::need_more_data);
    EXPECT_EQ(simd_parser->parse(" world\n"), ParseStatus::complete);
    EXPECT_EQ(simd_parser->fields(), std::vector<std::string>{"hello world"});
}

TEST_F(SimdParserTest, Buffer_IncompleteQuotedField) {
    EXPECT_EQ(simd_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(simd_parser->parse("lo\"\n"), ParseStatus::complete);
    EXPECT_EQ(simd_parser->fields(), std::vector<std::string>{"\"hello\""});
}

TEST_F(SimdParserTest, Buffer_QuoteAtBufferEnd_FollowedByNewline) {
    EXPECT_EQ(simd_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(simd_parser->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(simd_parser->fields(), std::vector<std::string>{"\"hello\""});
}

TEST_F(SimdParserTest, SimpleParsing_CorrectQuoting_NeedMoreDataWithLastCharAsQuote) {
    ExpectParse(simd_parser,  "\"something\"",
        ParseStatus::need_more_data, std::vector<std::string>{"\"something\""});
    ExpectParse(simd_parser,  "\"different\"",
        ParseStatus::need_more_data, std::vector<std::string>{"\"something\"\"different\""});
    ExpectParse(simd_parser,  ",next\n",
        ParseStatus::complete, std::vector<std::string>{"\"something\"\"different\"", "next"});
}

TEST_F(SimdParserTest, SimpleParsing_NewlineAndDelimiterInQuotes_ParserDoesntClearFieldsOnItOwn) {
    ExpectParse(simd_parser, "\"something",
        ParseStatus::need_more_data, std::vector<std::string>{"\"something"});

    EXPECT_EQ(simd_parser->consumed(), 10);

    ExpectParse(simd_parser, "\n,\",different,\"",
        ParseStatus::complete, std::vector<std::string>{"\"something"});
    
    EXPECT_EQ(simd_parser->consumed(), 1);

    ExpectParse(simd_parser, ",\",different,\",next\"\n",
        ParseStatus::complete, std::vector<std::string>{"\"something", "", "\"", "different", "\"", "next\""});

    EXPECT_EQ(simd_parser->fields(), std::vector<std::string>({"\"something", "", "\"", "different", "\"", "next\""}));
}

TEST_F(SimdParserTest, Buffer_SplitEscapedQuote) {
    ExpectParse(simd_parser, "\"a\"", ParseStatus::need_more_data, std::vector<std::string>{"\"a\""});
    ExpectParse(simd_parser, "\"b\"\n", ParseStatus::complete, std::vector<std::string>{"\"a\"\"b\""});
    EXPECT_EQ(simd_parser->fields(), std::vector<std::string>{"\"a\"\"b\""});
}

TEST_F(SimdParserTest, Buffer_EmptyBuffer) {
    ExpectParse(simd_parser, "", ParseStatus::need_more_data, (std::vector<std::string>{}));
    EXPECT_EQ(simd_parser->consumed(), 0);
    EXPECT_EQ(simd_parser->fields(), (std::vector<std::string>{}));
}

TEST_F(SimdParserTest, Buffer_MultipleChunks) {
    ExpectParse(simd_parser, "a,", ParseStatus::need_more_data, (std::vector<std::string>{"a", ""}));
    ExpectParse(simd_parser, "b,", ParseStatus::need_more_data, (std::vector<std::string>{"a", "b", ""}));
    ExpectParse(simd_parser, "c\n", ParseStatus::complete, (std::vector<std::string>{"a", "b", "c"}));
    EXPECT_EQ(simd_parser->fields(), (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(SimdParserTest, EmptyBufferThenData_DoesNotCrashAndParsesNormally) {
    ExpectParse(simd_parser, "", ParseStatus::need_more_data, (std::vector<std::string>{}));
    ExpectParse(simd_parser, "a\n", ParseStatus::complete, (std::vector<std::string>{"a"}));
    EXPECT_EQ(simd_parser->fields(), (std::vector<std::string>{"a"}));
}

TEST_F(SimdParserTest, MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    ExpectParse(simd_parser, "a,b\nc,d\n", ParseStatus::complete, (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(simd_parser->consumed(), 4); // "a,b\n"

    simd_parser->reset();
    ExpectParse(simd_parser, "c,d\n", ParseStatus::complete, (std::vector<std::string>{"c","d"}));
}

TEST_F(SimdParserTest, TrailingDelimiter) {
    ExpectParse(simd_parser, "a,b,\n", ParseStatus::complete, (std::vector<std::string>{"a","b",""}));
}

TEST_F(SimdParserTest, DelimiterOnlyWithNewline) {
    ExpectParse(simd_parser, ",\n", ParseStatus::complete, (std::vector<std::string>{"",""}));
}

TEST_F(SimdParserTest, OnlyDelimiter) {
    ExpectParse(simd_parser, ",", ParseStatus::need_more_data, (std::vector<std::string>{"",""}));
}

TEST_F(SimdParserTest, EOF_NoNewline_LastRecordReturnedViaMoveFields) {
    ExpectParse(simd_parser, "a,b", ParseStatus::need_more_data,
                std::vector<std::string>{"a","b"});
    EXPECT_EQ(simd_parser->fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(SimdParserTest, EmptyLine_RecordIsEmptyVector) {
    ExpectParse(simd_parser, "\na\n", ParseStatus::complete, std::vector<std::string>{});
    EXPECT_EQ(simd_parser->consumed(), 1);
}


TEST_F(SimdParserTest, Buffer_SingleCharChunks) {
    for (char c : std::string("a,b\n")) {
        std::string s(1, c);
        auto status = simd_parser->parse(s);
        if (c == '\n') {
            EXPECT_EQ(status, ParseStatus::complete);
        } else {
            EXPECT_EQ(status, ParseStatus::need_more_data);
        }
    }
    EXPECT_EQ(simd_parser->fields(), (std::vector<std::string>{"a", "b"}));
}


// ============================================================
// CRLF (lenient) behavior
// ============================================================

TEST(SimdParserLineEndingTest, CRLF_Accepts_CRLF_Strips_CR) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("a,b\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
}

TEST(SimdParserLineEndingTest, CRLF_Accepts_LF_Only_As_Well_Lenient) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("a,b\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
}

TEST(SimdParserLineEndingTest, CRLF_EmptyLine_DoesNotCrash_AndConsumesOneRecord) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{}));
    EXPECT_EQ(p->consumed(), 2u);
}

TEST(SimdParserLineEndingTest, CRLF_EmptyLine_WithLFOnly_DoesNotCrash) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{}));
    EXPECT_EQ(p->consumed(), 1u);
}

TEST(SimdParserLineEndingTest, CRLF_ConsumesTwoBytes_ForCRLF) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(p->consumed(), 5u); // "a,b\r\n" = 5
}

TEST(SimdParserLineEndingTest, CRLF_MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});

    EXPECT_EQ(p->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(p->consumed(), 5u);

    p->reset();
    EXPECT_EQ(p->parse("c,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"c","d"}));
    EXPECT_EQ(p->consumed(), 5u);
}

TEST(SimdParserLineEndingTest, CRLF_PartialAcrossChunks_CRThenLF) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});

    EXPECT_EQ(p->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(p->parse("\n"), ParseStatus::complete);

    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
}

// ============================================================
// CR-only mode (if you support it)
// ============================================================

TEST(SimdParserLineEndingTest, CR_Mode_Parses_CR_Terminated_Line) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::cr});
    EXPECT_EQ(p->parse("a,b\rc,d\r"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(p->consumed(), 4u);
}

TEST(SimdParserLineEndingTest, CR_Mode_DoesNotTreat_LF_AsTerminator) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::cr});
    EXPECT_EQ(p->parse("a,b\n"), ParseStatus::need_more_data);
    EXPECT_EQ(p->consumed(), 4u);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a","b\n"}));
}

// ============================================================
// Regression: no UB when newline not found
// (this catches the "compute newline_pos before nullptr-check" bug)
// ============================================================

TEST(SimdParserLineEndingTest, Regression_NoNewlinePtr_Nullptr_IsHandled) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});
    EXPECT_EQ(p->parse("abc"), ParseStatus::need_more_data);
    EXPECT_EQ(p->consumed(), 3u);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"abc"}));
}

TEST(SimdParserLineEndingTest, CRLF_SplitAcrossChunks_CRThenLF_StripsCR) {
    auto p = make_parser({.has_quoting = false, .use_simd = true, .line_ending = Config::LineEnding::crlf});

    EXPECT_EQ(p->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a", "b\r"}));

    EXPECT_EQ(p->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(p->fields(), (std::vector<std::string>{"a", "b"}));
}
