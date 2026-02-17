#include <iostream>
#include <cstring>
#include <csvparser/csvparser.hpp>
#include <algorithm>

namespace csv {

// --- Parser ---

template <typename FieldType>
ParserBase<FieldType>::ParserBase(const Config& config): config_(config) {}

template <typename FieldType>
void ParserBase<FieldType>::reset() noexcept {
    incomplete_last_read_ = false;
    pending_cr_ = false;
    fields_.clear();
    consumed_ = 0;
    err_msg_.clear();
}

template <typename FieldType>
size_t ParserBase<FieldType>::consumed() const noexcept {
    return consumed_;
}

template <typename FieldType>
std::string_view ParserBase<FieldType>::err_msg() const noexcept {
    return err_msg_;
}

template <typename FieldType>
std::vector<FieldType>& ParserBase<FieldType>::fields() noexcept {
    return fields_;
}


// --- Quoting Parser ---

template <typename FieldType>
QuotingParser<FieldType>::QuotingParser(const Config& config): Parser<FieldType>(config) {}

template <typename FieldType>
void QuotingParser<FieldType>::reset() noexcept {
    ParserBase<FieldType>::reset();
    in_quotes_ = false;
    pending_quote_ = false;
}

template <typename FieldType>
bool QuotingParser<FieldType>::is_quote(char c) {
    return c == this->config_.quote_char;
};

template <typename FieldType>
bool QuotingParser<FieldType>::is_delim(char c) {
    return c == this->config_.delimiter;
};

template <typename FieldType>
bool QuotingParser<FieldType>::is_newline(char c) {
    return this->config_.is_line_ending(c);
};

std::unique_ptr<Parser<std::string>> make_parser(const Config& config) {
    if (config.has_quoting) {
        if (config.parse_mode == Config::ParseMode::strict) {
            return std::make_unique<StrictQuotingParser>(config);
        }
        return std::make_unique<LenientQuotingParser>(config);
    }
    return std::make_unique<SimpleParser>(config);
}

template class ParserBase<std::string>;
template class ParserBase<std::string_view>;
template class QuotingParser<std::string>;
template class QuotingParser<std::string_view>;

}