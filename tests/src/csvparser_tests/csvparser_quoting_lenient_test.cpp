#include <gtest/gtest.h>

#include <csvparser/csvparser.hpp>
#include <csvconfig.hpp>

using namespace csv;

class LenientParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser> lenient_parser = make_parser({.parse_mode = Config::ParseMode::lenient});
    std::unique_ptr<Parser> semi_parser = make_parser({.delimiter = ';', .parse_mode = Config::ParseMode::lenient});
    std::unique_ptr<Parser> lenient_parser_crlf = make_parser({
        .parse_mode = Config::ParseMode::lenient,
        .line_ending = Config::LineEnding::crlf
    });
    std::unique_ptr<Parser> lenient_parser_cr = make_parser({
        .parse_mode = Config::ParseMode::lenient,
        .line_ending = Config::LineEnding::cr
    });
    std::unique_ptr<Parser> lenient_parser_lf = make_parser({
        .parse_mode = Config::ParseMode::lenient,
        .line_ending = Config::LineEnding::lf
    });
    void ExpectParse(std::unique_ptr<Parser>& parser,
                std::string_view input,
                ParseStatus expected_status,
                const std::vector<std::string>& expected_fields = {}) {
        EXPECT_EQ(parser->parse(input), expected_status);
        if (!expected_fields.empty()) {
            EXPECT_EQ(parser->peek_fields(), expected_fields);
        }
    }
};

// ============================================================
// BASIC PARSING - HAPPY PATH
// ============================================================

TEST_F(LenientParserTest, Basic_SingleField) {
    ExpectParse(lenient_parser, "hello\n", 
        ParseStatus::complete, {"hello"});
}

TEST_F(LenientParserTest, Basic_MultipleFields) {
    ExpectParse(lenient_parser, "a,b,c\n", 
        ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(LenientParserTest, Basic_EmptyFields) {
    ExpectParse(lenient_parser, "a,,c\n", 
        ParseStatus::complete, {"a", "", "c"});
}

TEST_F(LenientParserTest, Basic_AllEmptyFields) {
    ExpectParse(lenient_parser, ",,\n", 
        ParseStatus::complete, {"", "", ""});
}

TEST_F(LenientParserTest, Basic_SingleEmptyField) {
    ExpectParse(lenient_parser, "\n", 
        ParseStatus::complete, {""});
}

// ============================================================
// MOVE FIELDS
// ============================================================

TEST_F(LenientParserTest, MoveFields_MoveData) {
    const std::string data = "\"Mark\",is,quite,\"\"\"normal\"\"\"\n";
    std::vector<std::string> expected_fields = {"Mark", "is", "quite", "\"normal\""};

    ExpectParse(lenient_parser, data, ParseStatus::complete, expected_fields);
    EXPECT_EQ(lenient_parser->move_fields(), expected_fields);
    EXPECT_TRUE(lenient_parser->peek_fields().empty());
    EXPECT_TRUE(lenient_parser->move_fields().empty());
}

TEST_F(LenientParserTest, MoveFields_EmptyFields) {
    EXPECT_TRUE(lenient_parser->move_fields().empty());
    EXPECT_TRUE(lenient_parser->peek_fields().empty());
}

// ============================================================
// LENIENT QUOTING BEHAVIOR
// ============================================================

TEST_F(LenientParserTest, Lenient_QuoteInUnquotedField_TreatedAsLiteral) {
    // aa"hello"a -> aa"hello"a (quotes are literal, not in quoted mode)
    ExpectParse(lenient_parser, "aa\"hello\"a\n",
                ParseStatus::complete, {"aa\"hello\"a"});
}

TEST_F(LenientParserTest, Lenient_DoubleQuoteInUnquotedField_TreatedAsLiteral) {
    // aa""hello"a -> aa""hello"a (all quotes literal)
    ExpectParse(lenient_parser, "aa\"\"hello\"a\n",
                ParseStatus::complete, {"aa\"\"hello\"a"});
}

TEST_F(LenientParserTest, Lenient_QuotedFieldWithContentAfterClose) {
    // "aa"hello"a -> aahello"a (quoted start, close after aa, rest is literal)
    ExpectParse(lenient_parser, "\"aa\"hello\"a\n",
                ParseStatus::complete, {"aahello\"a"});
}

TEST_F(LenientParserTest, Lenient_EmptyQuotedFollowedByContent) {
    // ""aahello -> aahello (empty quoted field "" then literal content? OR just aahello)
    // Based on our rules: " opens, " closes (empty), aahello is literal after
    ExpectParse(lenient_parser, "\"\"aahello\n",
                ParseStatus::complete, {"aahello"});
}

TEST_F(LenientParserTest, Lenient_QuoteInMiddleOfUnquotedField) {
    ExpectParse(lenient_parser, "hel\"lo\n", 
                ParseStatus::complete, {"hel\"lo"});
}

TEST_F(LenientParserTest, Lenient_ContentAfterClosingQuote) {
    ExpectParse(lenient_parser, "\"hello\"world\n", 
                ParseStatus::complete, {"helloworld"});
}

TEST_F(LenientParserTest, Lenient_QuoteAfterContent) {
    ExpectParse(lenient_parser, "hello\",world\n", 
                ParseStatus::complete, {"hello\"", "world"});
}

TEST_F(LenientParserTest, Lenient_SpaceBeforeQuote) {
    // Space before quote = unquoted field, quote is literal
    ExpectParse(lenient_parser, " \"hello\"\n", 
                ParseStatus::complete, {" \"hello\""});
}

TEST_F(LenientParserTest, Lenient_SpaceAfterQuote) {
    // "hello" followed by space = content after closing quote
    ExpectParse(lenient_parser, "\"hello\" \n", 
                ParseStatus::complete, {"hello "});
}

TEST_F(LenientParserTest, Lenient_MalformedQuoteInUnquotedField) {
    ExpectParse(lenient_parser, R"(aa"ada","normal")",
                ParseStatus::need_more_data, {"aa\"ada\"", "normal"});
}

TEST_F(LenientParserTest, Lenient_ContentAfterEscapedQuote) {
    // "something""different"here -> something"differenthere
    ExpectParse(lenient_parser, "\"something\"\"different\"here,next\n",
                ParseStatus::complete, {"something\"differenthere", "next"});
}

TEST_F(LenientParserTest, Lenient_CorrectQuoting_NoContentAfterClosingQuote) {
    ExpectParse(lenient_parser, "\"something\"\"different\",next\n",
        ParseStatus::complete, {"something\"different", "next"});
}

// ============================================================
// STANDARD QUOTING (VALID CSV)
// ============================================================

TEST_F(LenientParserTest, Quoted_SimpleField) {
    ExpectParse(lenient_parser, "\"hello\"\n", 
                ParseStatus::complete, {"hello"});
}

TEST_F(LenientParserTest, Quoted_FieldWithComma) {
    ExpectParse(lenient_parser, "\"hello,world\"\n", 
                ParseStatus::complete, {"hello,world"});
}

TEST_F(LenientParserTest, Quoted_FieldWithNewline) {
    ExpectParse(lenient_parser, "\"hello\nworld\"\n", 
                ParseStatus::complete, {"hello\nworld"});
}

TEST_F(LenientParserTest, Quoted_EscapedQuote) {
    ExpectParse(lenient_parser, "\"hello\"\"world\"\n", 
                ParseStatus::complete, {"hello\"world"});
}

TEST_F(LenientParserTest, Quoted_OnlyEscapedQuote) {
    ExpectParse(lenient_parser, "\"\"\"\"\n", 
                ParseStatus::complete, {"\""});
}

TEST_F(LenientParserTest, Quoted_MultipleEscapedQuotes) {
    ExpectParse(lenient_parser, "\"\"\"\"\"\"\n", 
                ParseStatus::complete, {"\"\""});
}

TEST_F(LenientParserTest, Quoted_EmptyQuotedField) {
    ExpectParse(lenient_parser, "\"\"\n", 
                ParseStatus::complete, {""});
}

TEST_F(LenientParserTest, Quoted_MixedQuotedAndUnquoted) {
    ExpectParse(lenient_parser, "a,\"b,c\",d\n", 
                ParseStatus::complete, {"a", "b,c", "d"});
}

TEST_F(LenientParserTest, Quoted_QuotedFieldAtStart) {
    ExpectParse(lenient_parser, "\"a\",b,c\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(LenientParserTest, Quoted_QuotedFieldAtEnd) {
    ExpectParse(lenient_parser, "a,b,\"c\"\n", 
                ParseStatus::complete, {"a", "b", "c"});
}

TEST_F(LenientParserTest, Quoted_LiteralQuotesInQuotedField) {
    ExpectParse(lenient_parser, "\"Mark\",is,quite,\"\"normal\"\"\n", 
                ParseStatus::complete, {"Mark", "is", "quite", "normal\"\""});
}

TEST_F(LenientParserTest, Quoted_ComplexQuoting) {
    // """normal""" = " opens, "" escape (literal "), normal, "" escape (literal "), " closes
    ExpectParse(lenient_parser, "\"Mark\",is,quite,\"\"\"normal\"\"\"\n", 
                ParseStatus::complete, {"Mark", "is", "quite", "\"normal\""});
}

// ============================================================
// PARTIAL PARSING / BUFFER HANDLING
// ============================================================

TEST_F(LenientParserTest, Buffer_IncompleteUnquotedField) {
    EXPECT_EQ(lenient_parser->parse("hello"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse(" world\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"hello world"});
}

TEST_F(LenientParserTest, Buffer_IncompleteQuotedField) {
    EXPECT_EQ(lenient_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("lo\"\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"hello"});
}

TEST_F(LenientParserTest, Buffer_IncompleteQuotedField_QuoteInTheMiddle) {
    EXPECT_EQ(lenient_parser->parse("\"hel"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("lo\"!\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"hello!"});
}

TEST_F(LenientParserTest, Buffer_IncompleteQuotedField_QuoteAtTheEnd) {
    EXPECT_EQ(lenient_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("\""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"hello\""});
}

TEST_F(LenientParserTest, Buffer_QuoteAtBufferEnd_FollowedByNewline) {
    EXPECT_EQ(lenient_parser->parse("\"hello\""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"hello"});
}

TEST_F(LenientParserTest, Buffer_QuoteAtBufferEnd_FollowedByQuote) {
    // "something" + "different" across buffer = something"different
    ExpectParse(lenient_parser, "\"something\"", ParseStatus::need_more_data);
    ExpectParse(lenient_parser, "\"different\"", ParseStatus::need_more_data);
    ExpectParse(lenient_parser, ",next\n", ParseStatus::complete, 
                {"something\"different", "next"});
}

TEST_F(LenientParserTest, Buffer_NewlineAndDelimiterInQuotes) {
    ExpectParse(lenient_parser, "\"something", ParseStatus::need_more_data, {"something"});
    ExpectParse(lenient_parser, "\n,\",different,\"", ParseStatus::need_more_data, 
                {"something\n,", "different", ""});
    ExpectParse(lenient_parser, ",next\"\n", ParseStatus::complete, 
                {"something\n,", "different", ",next"});

    std::vector<std::string> expected_fields = {"something\n,", "different", ",next"};
    EXPECT_EQ(lenient_parser->move_fields(), expected_fields);
}

TEST_F(LenientParserTest, Buffer_SplitEscapedQuote) {
    EXPECT_EQ(lenient_parser->parse("\"a\""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("\"b\"\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"a\"b"});
}

TEST_F(LenientParserTest, Buffer_EmptyBuffer) {
    EXPECT_EQ(lenient_parser->parse(""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->consumed(), 0);
}

TEST_F(LenientParserTest, Buffer_MultipleChunks) {
    EXPECT_EQ(lenient_parser->parse("a,"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("b,"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("c\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(LenientParserTest, Buffer_SingleCharChunks) {
    for (char c : std::string("a,b\n")) {
        std::string s(1, c);
        auto status = lenient_parser->parse(s);
        if (c == '\n') {
            EXPECT_EQ(status, ParseStatus::complete);
        } else {
            EXPECT_EQ(status, ParseStatus::need_more_data);
        }
    }
    EXPECT_EQ(lenient_parser->move_fields(), (std::vector<std::string>{"a", "b"}));
}

// ============================================================
// RESET FUNCTIONALITY
// ============================================================

TEST_F(LenientParserTest, Reset_ClearsFields) {
    std::vector<std::string> expected_fields = {"a", "b"};
    ExpectParse(lenient_parser, "a,b\nabc", ParseStatus::complete, expected_fields);
    EXPECT_EQ(lenient_parser->peek_fields(), expected_fields);
    lenient_parser->reset();
    EXPECT_EQ(lenient_parser->peek_fields(), std::vector<std::string>{});
}

TEST_F(LenientParserTest, Reset_ClearsState) {
    lenient_parser->parse("\"hello");  // In quotes
    lenient_parser->reset();
    // Should parse fresh, not continue quoted state
    EXPECT_EQ(lenient_parser->parse("world\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"world"});
}

TEST_F(LenientParserTest, Reset_ClearsPendingQuote) {
    lenient_parser->parse("\"hello\"");  // Pending quote
    lenient_parser->reset();
    EXPECT_EQ(lenient_parser->parse("world\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), std::vector<std::string>{"world"});
}

TEST_F(LenientParserTest, Reset_ClearsConsumed) {
    lenient_parser->parse("hello\n");
    lenient_parser->reset();
    EXPECT_EQ(lenient_parser->consumed(), 0);
}

// ============================================================
// EDGE CASES
// ============================================================

TEST_F(LenientParserTest, Edge_OnlyNewline) {
    ExpectParse(lenient_parser, "\n", ParseStatus::complete, {""});
}

TEST_F(LenientParserTest, Edge_OnlyDelimiter) {
    ExpectParse(lenient_parser, ",", ParseStatus::need_more_data, {"", ""});
}

TEST_F(LenientParserTest, Edge_DelimiterThenNewline) {
    ExpectParse(lenient_parser, ",\n", ParseStatus::complete, {"", ""});
}

TEST_F(LenientParserTest, Edge_ManyEmptyFields) {
    ExpectParse(lenient_parser, ",,,,\n", ParseStatus::complete, {"", "", "", "", ""});
}

TEST_F(LenientParserTest, Edge_QuotedEmpty) {
    ExpectParse(lenient_parser, "\"\",\"\"\n", ParseStatus::complete, {"", ""});
}

TEST_F(LenientParserTest, Edge_VeryLongField) {
    std::string long_field(10000, 'a');
    std::string input = long_field + "\n";
    ExpectParse(lenient_parser, input, ParseStatus::complete, {long_field});
}

TEST_F(LenientParserTest, Edge_VeryLongQuotedField) {
    std::string long_field(10000, 'a');
    std::string input = "\"" + long_field + "\"\n";
    ExpectParse(lenient_parser, input, ParseStatus::complete, {long_field});
}

// ============================================================
// SPECIFIC LENIENT CASES
// ============================================================

TEST_F(LenientParserTest, Lenient_Example1_UnquotedWithQuotes) {
    // aa""hello"a -> aa""hello"a (not quoted, all literal)
    ExpectParse(lenient_parser, "aa\"\"hello\"a\n",
                ParseStatus::complete, {"aa\"\"hello\"a"});
}

TEST_F(LenientParserTest, Lenient_Example2_QuotedWithContentAfter) {
    // "aa"hello"a -> aahello"a 
    // " opens, aa content, " closes, hello"a literal after
    ExpectParse(lenient_parser, "\"aa\"hello\"a\n",
                ParseStatus::complete, {"aahello\"a"});
}

TEST_F(LenientParserTest, Lenient_Example3_EmptyQuotedThenContent) {
    // ""aahello -> aahello
    // " opens, " closes (empty), aahello literal after
    ExpectParse(lenient_parser, "\"\"aahello\n",
                ParseStatus::complete, {"aahello"});
}

TEST_F(LenientParserTest, Lenient_EmptyQuotedThenQuotedContent) {
    // ""aa""hello -> aa"hello
    // " opens, " closes (empty), aa literal, "" literal, hello literal
    // Wait no... after close we're unquoted, so "" is just ""
    // Result: aa""hello
    ExpectParse(lenient_parser, "\"\"aa\"\"hello\n",
                ParseStatus::complete, {"aa\"\"hello"});
}

TEST_F(LenientParserTest, Lenient_MultipleQuotedSections) {
    // "a"b"c" -> abc
    // " opens, a content, " closes, b literal, " opens? NO - not at field start
    // So: " opens, a, " closes, b"c" all literal
    ExpectParse(lenient_parser, "\"a\"b\"c\"\n",
                ParseStatus::complete, {"ab\"c\""});
}

TEST_F(LenientParserTest, Lenient_QuotedCommaAfterClose) {
    // "hello",world" -> hello (field 1), world" (field 2)
    ExpectParse(lenient_parser, "\"hello\",world\"\n",
                ParseStatus::complete, {"hello", "world\""});
}

TEST_F(LenientParserTest, Lenient_ComplexMixedQuoting) {
    // a,"b"c,d"e","f" -> a | bc | d"e" | f
    // Field 1: a (unquoted)
    // Field 2: "b"c -> " opens, b, " closes, c literal = bc
    // Field 3: d"e" (unquoted, quotes literal)
    // Field 4: "f" -> f
    ExpectParse(lenient_parser, "a,\"b\"c,d\"e\",\"f\"\n",
                ParseStatus::complete, {"a", "bc", "d\"e\"", "f"});
}

TEST_F(LenientParserTest, Lenient_OnlyQuotes) {
    // " -> opens, "" -> literal, \n -> literal, unclosed at end (need more data)
    ExpectParse(lenient_parser, "\"\"\"\n",
                ParseStatus::need_more_data, {"\"\n"});
}

TEST_F(LenientParserTest, Lenient_FourQuotes) {
    // """" -> " opens, "" escape = ", " closes = "
    ExpectParse(lenient_parser, "\"\"\"\"\n",
                ParseStatus::complete, {"\""});
}

TEST_F(LenientParserTest, Lenient_FiveQuotes) {
    // """"" -> " opens, "" escape, "" escape? no only one " left
    // " opens, "", " closes, " literal after
    // = " + "
    // Actually: " opens, "" = escape ("), " closes, " literal = ""
    ExpectParse(lenient_parser, "\"\"\"\"\"\n",
                ParseStatus::need_more_data, {"\"\"\n"});
}

TEST_F(LenientParserTest, Lenient_SixQuotes) {
    // """""" -> " opens, "" escape, "" escape, " closes = ""
    ExpectParse(lenient_parser, "\"\"\"\"\"\"\n",
                ParseStatus::complete, {"\"\""});
}

TEST_F(LenientParserTest, Lenient_UnclosedQuoteAtEOF) {
    // "hello without close - lenient accepts at EOF
    EXPECT_EQ(lenient_parser->parse("\"hello"), ParseStatus::need_more_data);
    // Simulate EOF by checking what we have
    EXPECT_EQ(lenient_parser->peek_fields(), std::vector<std::string>{"hello"});
}

TEST_F(LenientParserTest, Lenient_NewlineInQuotedField) {
    ExpectParse(lenient_parser, "\"line1\nline2\"\n",
                ParseStatus::complete, {"line1\nline2"});
}

TEST_F(LenientParserTest, Lenient_CarriageReturnInQuotedField) {
    ExpectParse(lenient_parser, "\"line1\r\nline2\"\n",
                ParseStatus::complete, {"line1\r\nline2"});
}

TEST_F(LenientParserTest, Lenient_TabsAndSpaces) {
    ExpectParse(lenient_parser, "  a  ,  b  ,  c  \n",
                ParseStatus::complete, {"  a  ", "  b  ", "  c  "});
}

TEST_F(LenientParserTest, Lenient_QuotedTabsAndSpaces) {
    ExpectParse(lenient_parser, "\"  a  \",\"  b  \"\n",
                ParseStatus::complete, {"  a  ", "  b  "});
}

// ============================================================
// CR-only mode
// ============================================================

TEST_F(LenientParserTest, CR_Mode_Parses_CR_Terminated_Line) {
    EXPECT_EQ(lenient_parser_cr->parse("a,b\rc,d\r"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_cr->peek_fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(lenient_parser_cr->consumed(), 4u);
}

TEST_F(LenientParserTest, CR_Mode_DoesNotTreat_LF_AsTerminator) {
    EXPECT_EQ(lenient_parser_cr->parse("a,b\n"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_cr->consumed(), 4u);
    EXPECT_EQ(lenient_parser_cr->peek_fields(), (std::vector<std::string>{"a","b\n"}));
}

// ============================================================
// LF-only mode
// ============================================================

TEST_F(LenientParserTest, LF_Mode_DoesNotTreat_CR_AsTerminator) {
    EXPECT_EQ(lenient_parser_lf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_lf->consumed(), 4u);
    EXPECT_EQ(lenient_parser_lf->peek_fields(), (std::vector<std::string>{"a","b\r"}));
}

// ============================================================
// CRLF (lenient) behavior
// ============================================================

TEST_F(LenientParserTest, CRLF_Accepts_CRLF_Strips_CR) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(LenientParserTest, CRLF_AcceptsLFOnly) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(LenientParserTest, CRLF_EmptyLine_DoesNotCrash_AndConsumesOneRecord) {
    EXPECT_EQ(lenient_parser_crlf->parse("\r\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{""}));
    EXPECT_EQ(lenient_parser_crlf->consumed(), 2u);
}

TEST_F(LenientParserTest, CRLF_EmptyLine_WithLFOnly_Complete) {
    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{""}));
}

TEST_F(LenientParserTest, CRLF_ConsumesTwoBytes_ForCRLF) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(lenient_parser_crlf->consumed(), 5u);
}

TEST_F(LenientParserTest, CRLF_MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r\nc,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a","b"}));
    EXPECT_EQ(lenient_parser_crlf->consumed(), 5u);

    lenient_parser_crlf->reset();
    EXPECT_EQ(lenient_parser_crlf->parse("c,d\r\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"c","d"}));
    EXPECT_EQ(lenient_parser_crlf->consumed(), 5u);
}

TEST_F(LenientParserTest, CRLF_PartialAcrossChunks_CRThenLF) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a","b\r"}));

    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a","b"}));
}

// ============================================================
// CRLF Tests
// ============================================================

TEST_F(LenientParserTest, Regression_NoNewlinePtr_Nullptr_IsHandled) {
    EXPECT_EQ(lenient_parser_crlf->parse("abc"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->consumed(), 3u);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"abc"}));
}

TEST_F(LenientParserTest, CRLF_SplitAcrossChunks_CRThenLF_StripsCR) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a", "b\r"}));

    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a", "b"}));
}

TEST_F(LenientParserTest, CRLF_SplitAfterClosingQuote) {
    EXPECT_EQ(lenient_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a\r"}));

    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a"}));
}

TEST_F(LenientParserTest, PendingCR_PopsWithoutChecks) {
    EXPECT_EQ(lenient_parser_crlf->parse("a\r"), ParseStatus::need_more_data);
    lenient_parser_crlf->reset();
    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete); // pending_cr_ is false after reset()
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{""}));
}

TEST_F(LenientParserTest, NeedMoreDataMustConsumeOrProgress) {
    EXPECT_EQ(lenient_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->consumed(), 4u);
}

TEST_F(LenientParserTest, CRLF_SplitAcrossBuffers_OutsideQuotes) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);

    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(LenientParserTest, CRLF_QuoteThenCRAtEnd_ProgressAndCorrectConsume) {
    EXPECT_EQ(lenient_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->consumed(), 4u);
    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a"}));
}

TEST_F(LenientParserTest, SplitOutsideQuotes_CRThenLF_RemovesCR) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a", "b\r"}));
    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(LenientParserTest, SplitOutsideQuotes_CRThenChar_TreatsCR_AsData_Complete) {
    EXPECT_EQ(lenient_parser_crlf->parse("a,b\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a", "b\r"}));
    EXPECT_EQ(lenient_parser_crlf->parse("a\r\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a","b\ra"}));
}

TEST_F(LenientParserTest, EmptyLineSplit_CRThenLF_ProducesEmptyRecord) {
    EXPECT_EQ(lenient_parser_crlf->parse("\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{""}));
}

TEST_F(LenientParserTest, PendingCR_NotFollowedByLF_TreatsAsData) {
    EXPECT_EQ(lenient_parser_crlf->parse("a\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->parse("x"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->peek_fields(), (std::vector<std::string>{"a\rx"}));
}

TEST_F(LenientParserTest, ClosingQuoteThenCRLF_InSameBuffer) {
    EXPECT_EQ(lenient_parser_crlf->parse("\"a\"\r\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a"}));
}

TEST_F(LenientParserTest, ClosingQuoteThenCR_SplitThenLF_Completes) {
    EXPECT_EQ(lenient_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->parse("\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a"}));
}

TEST_F(LenientParserTest, CRatTheEndOfBuffer_needMoreData_OnlyDataWithCRasData) {
    EXPECT_EQ(lenient_parser_crlf->parse("\"a\"\r"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser_crlf->move_fields(), (std::vector<std::string>{"a\r"}));
}

// ============================================================
// Additional Tests
// ============================================================

TEST_F(LenientParserTest, EOF_NoNewline_Unquoted_ReturnsNeedMoreData) {
    ExpectParse(lenient_parser, "a,b,c", ParseStatus::need_more_data, {"a","b","c"});
}

TEST_F(LenientParserTest, EOF_NoNewline_QuotedClosed_ReturnsNeedMoreData) {
    ExpectParse(lenient_parser, "\"a\",\"b\"", ParseStatus::need_more_data, {"a","b"});
}

TEST_F(LenientParserTest, TrailingDelimiter_EndOfBuffer_NeedMoreData_IncludesEmptyField) {
    ExpectParse(lenient_parser, "a,", ParseStatus::need_more_data, {"a",""});
}

TEST_F(LenientParserTest, TrailingDelimiter_BeforeNewline_Completes_IncludesEmptyField) {
    ExpectParse(lenient_parser, "a,\n", ParseStatus::complete, {"a",""});
}

TEST_F(LenientParserTest, Buffer_SplitEmptyQuotedField) {
    EXPECT_EQ(lenient_parser->parse("\""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("\"\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), (std::vector<std::string>{""}));
}

TEST_F(LenientParserTest, Buffer_Split_EmptyQuotedField_ThenDelimiter) {
    EXPECT_EQ(lenient_parser->parse("\"\""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse(",x\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), (std::vector<std::string>{"", "x"}));
}

TEST_F(LenientParserTest, Buffer_Split_AfterDelimiter_BeforeOpenQuote) {
    EXPECT_EQ(lenient_parser->parse("a,"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse("\"b\"\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(LenientParserTest, Buffer_ClosingQuoteAtEnd_FollowedByDelimiter) {
    EXPECT_EQ(lenient_parser->parse("\"a\""), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->parse(",b\n"), ParseStatus::complete);
    EXPECT_EQ(lenient_parser->move_fields(), (std::vector<std::string>{"a","b"}));
}

TEST_F(LenientParserTest, Lenient_SpaceBeforeQuote_AfterDelimiter_Complete) {
    ExpectParse(lenient_parser, "a, \"b\"\n", ParseStatus::complete, (std::vector<std::string>{"a"," \"b\""}));
}

TEST_F(LenientParserTest, Lenient_GarbageAfterDelimiterBeforeQuote_Complete) {
    ExpectParse(lenient_parser, "a,x\"b\"\n", ParseStatus::complete, (std::vector<std::string>{"a","x\"b\""}));
}

TEST_F(LenientParserTest, LF_MultipleRecordsInOneBuffer_ConsumesOnlyFirst) {
    ExpectParse(lenient_parser, "a,b\nc,d\n", ParseStatus::complete, {"a","b"});
    EXPECT_EQ(lenient_parser->consumed(), 4u); // "a,b\n"
}

TEST_F(LenientParserTest, CRLF_CRInsideQuotes_IsData) {
    ExpectParse(lenient_parser_crlf, "\"a\rb\"\r\n", ParseStatus::complete, {"a\rb"});
}

TEST_F(LenientParserTest, CRLF_QuotedFieldThenDelimiterThenCRLF) {
    ExpectParse(lenient_parser_crlf, "\"a\",b\r\n", ParseStatus::complete, {"a","b"});
}

TEST_F(LenientParserTest, NeedMoreData_ConsumedCountsBytes_LF) {
    EXPECT_EQ(lenient_parser->parse("ab"), ParseStatus::need_more_data);
    EXPECT_EQ(lenient_parser->consumed(), 2u);
}

TEST_F(LenientParserTest, Quoted_FieldStartsWithDelimiterChar) {
    ExpectParse(lenient_parser, "\",\"\n", ParseStatus::complete, {","});
}

TEST_F(LenientParserTest, Quoted_FieldStartsWithNewlineChar) {
    ExpectParse(lenient_parser, "\"\n\"\n", ParseStatus::complete, {"\n"});
}