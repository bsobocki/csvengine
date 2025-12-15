#include <csvparser.hpp>
#include <iostream>

using namespace csv;

namespace {
constexpr char quoting_char = '\"';

std::vector<std::string_view> split(std::string_view str, const char delim) {
    std::vector<std::string_view> result;

    size_t start = 0;
    size_t end = str.find(delim);

    while(end != std::string_view::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delim, start);
    }

    result.push_back(str.substr(start));

    return result;
}
}

Parser::Parser(Config config): config_(config) {   
}

Parser::ParseStatus Parser::parse(std::string_view buffer) {
    if (config_.has_quoting) {
        if (config_.parse_mode == Config::ParseMode::strict) {
            return csv_quotes_strict_parse(buffer);
        }
        return csv_quotes_lenient_parse(buffer);
    } 
    return naive_parse(buffer);
}

// === simplest csv quotes parsing - char by char ===

// strict mode
Parser::ParseStatus Parser::csv_quotes_strict_parse(std::string_view buffer) {
    auto ch = buffer.begin();
    auto field_start = ch;
    consumed_ = 0;

    auto is_beg =     [&](auto ch) { return ch == buffer.begin(); };
    auto is_end =     [&](auto ch) { return ch == buffer.end(); };
    auto is_quote =   [&](auto ch) { return !is_end(ch) && *ch == quoting_char; };
    auto is_delim =   [&](auto ch) { return *ch == config_.delimiter; };
    auto is_newline = [&](auto ch) { return config_.is_line_ending(*ch); };
    auto add_field =  [&](auto ch) { 
        fields_.emplace_back(field_start, ch);
        consumed_ += std::distance(field_start, ch);
    };

    while (!is_end(ch)) {
        if (is_newline(ch) && !in_quotes_) {
            consumed_++;
            add_field(ch);
            return ParseStatus::complete;
        }

        if (is_delim(ch) && !in_quotes_) {
            consumed_++;
            add_field(ch);
            field_start = ch+1;
        }
        else if (is_quote(ch)) {
            // double quote => literal
            if (!is_end(ch+1) && is_quote(ch+1)) {
                ch++; // skip one char to make literal
            }
            // one quote => quoting
            else if (in_quotes_) {
                if (!is_end(ch+1)) {
                    // wrong quoting => data after quotes
                    if (!is_delim(ch+1) && !is_newline(ch+1)) {
                        std::cout << "parse failed as end quote in wrong place! " <<std::endl;
                        return ParseStatus::fail;
                    }

                    add_field(ch);
                    field_start = ch+2; // next field after quote and delim (for newline we will return status complete)
                    consumed_++; // consume also this char, even though we don't include it in field

                    if (is_delim(ch+1)) {
                        ch++; // skip delim to add only one field
                    }
                    else if (is_newline(ch+1)) {
                        return ParseStatus::complete;
                    }
                }
                in_quotes_ = false;
            }
            else if (is_beg(ch) || (!is_beg(ch) && is_delim(ch-1))) {
                in_quotes_ = true;
                field_start++;
            }
            else {
                std::cout << "parse failed as start quote in wrong place! " <<std::endl;
                return ParseStatus::fail;
            }
        }
        // skip every other character 
        ch++;
    }

    add_field(ch);
    is_last_field_not_full_ = true;

    std::cout << "parse need more data! " <<std::endl;
    return Parser::ParseStatus::need_more_data;
}

// lenient mode
Parser::ParseStatus Parser::csv_quotes_lenient_parse(std::string_view buffer) {
    // TODO: implement lenient parse
    return csv_quotes_strict_parse(buffer);
}

Parser::ParseStatus Parser::naive_parse(std::string_view buffer) {
    // naive implementation without quotes
    auto newline_pos = buffer.find('\n');

    if (newline_pos == std::string_view::npos) {
        if (buffer.empty()) {
            is_last_field_not_full_ = true;
            return Parser::ParseStatus::need_more_data;
        }
        else {
            auto fields = split(buffer, config_.delimiter);
            insert_fields(fields);
            consumed_ += buffer.size();
        }
        
        return Parser::ParseStatus::need_more_data;
    }

    auto line = buffer.substr(0, newline_pos);

    if (line.empty()) {
        return Parser::ParseStatus::complete;
    }

    auto fields = split(line, config_.delimiter);
    insert_fields(fields);

    consumed_ += line.size()  + 1; // + newline character

    return Parser::ParseStatus::complete;
}

void Parser::insert_fields(const std::vector<std::string_view>& fields) {
    auto field = fields.begin();
    if (is_last_field_not_full_) {
        fields_[fields_.size()-1] += *field++;
        is_last_field_not_full_ = false;
    }

    while(field != fields.end()) {
        fields_.emplace_back(*field++);
    }
}

void Parser::reset() {
    in_quotes_ = false;
    fields_ = {};
    consumed_ = 0;
    err_msg_ = "";
}

size_t Parser::consumed() const {
    return consumed_;
}

std::string Parser::err_msg() const {
    return err_msg_;
}

std::vector<std::string> Parser::move_fields() const {
    return std::move(fields_);
}