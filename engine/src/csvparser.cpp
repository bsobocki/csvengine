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

void Parser::reset() {
    incomplete_last_read_ = false;
    fields_.clear();
    consumed_ = 0;
    err_msg_.clear();
}

void QuotingParser::reset() {
    Parser::reset();
    in_quotes_ = false;
    pending_quote_ = false;
}

size_t Parser::consumed() const {
    return consumed_;
}

std::string Parser::err_msg() const {
    return err_msg_;
}

std::vector<std::string> Parser::move_fields() {
    return std::move(fields_);
}

std::vector<std::string_view> Parser::peek_fields() {
    return std::vector<std::string_view>(fields_.begin(), fields_.end());
}

}