#include <iostream>
#include <cstring>
#include <csvparser.hpp>

namespace csv {

Parser::Parser(const Config& config): config_(config) {}

QuotingParser::QuotingParser(const Config& config): Parser(config) {}

std::unique_ptr<Parser> make_parser(const Config& config) {
    if (config.has_quoting) {
        if (config.parse_mode == Config::ParseMode::strict) {
            return std::make_unique<StrictQuotingParser>(config);
        }
        return std::make_unique<LenientQuotingParser>(config);
    }
    return std::make_unique<SimpleParser>(config);
}



// ============================================================
// Parser
// ============================================================

void Parser::reset() noexcept {
    incomplete_last_read_ = false;
    pending_cr_ = false;
    fields_.clear();
    consumed_ = 0;
    err_msg_.clear();
}

void QuotingParser::reset() noexcept {
    Parser::reset();
    in_quotes_ = false;
    pending_quote_ = false;
}

size_t Parser::consumed() const noexcept {
    return consumed_;
}

std::string_view Parser::err_msg() const noexcept {
    return err_msg_;
}

std::vector<std::string> Parser::move_fields() noexcept {
    return std::move(fields_);
}

const std::vector<std::string>& Parser::peek_fields() const noexcept {
    return fields_;
}



// ============================================================
// Quoting Parser
// ============================================================

void QuotingParser::remove_last_saved_char() {
    if (!fields_.empty() && !fields_.back().empty()) {
        fields_.back().pop_back();
    }
}

bool QuotingParser::is_quote(char c) {
    return c == config_.quote_char;
};

bool QuotingParser::is_delim(char c) {
    return c == config_.delimiter;
};

bool QuotingParser::is_newline(char c) {
    return config_.is_line_ending(c);
};

}