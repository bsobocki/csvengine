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
    consumed_ = 0;

    if (buffer.size() == 0) return ParseStatus::need_more_data;

    auto ch = buffer.begin();
    auto field_start = ch;
    size_t current_field_quote_literals = 0;    
    
    auto is_beg     = [&](auto ch) { return ch == buffer.begin(); };
    auto is_end     = [&](auto ch) { return ch == buffer.end(); };
    auto is_quote   = [&](auto ch) { return *ch == quoting_char; };
    auto is_delim   = [&](auto ch) { return *ch == config_.delimiter; };
    auto is_newline = [&](auto ch) { return config_.is_line_ending(*ch); };

    if (incomplete_with_quote_as_last_char_) {
        incomplete_with_quote_as_last_char_ = false;

        if (is_quote(ch)) {
            in_quotes_ = true;
            ch++;
        }
        else if (!is_delim(ch) && !is_newline(ch)) {
            return ParseStatus::fail;
        }

        if (is_newline(ch)) {
            return ParseStatus::complete;
        }

        if (is_delim(ch)) {
            ch++; // we already have the field
            field_start = ch;
            incomplete_last_read_ = false;
        }
    }

    auto add_field =  [&](auto ch) {
        if (!incomplete_last_read_) {
            fields_.emplace_back("");
        }

        std::string& field_ref = fields_.back();
        size_t new_part_idx = field_ref.size();

        size_t curr_field_size = std::distance(field_start, ch) - current_field_quote_literals;
        field_ref.resize(field_ref.size() + curr_field_size);

        auto it = field_start;
        while (it != ch) {
            field_ref[new_part_idx++] = *it++;
            // for double quotes just skip the next one
            if (is_quote(it) && is_quote(it-1)) it++;
        }
        
        current_field_quote_literals = 0;
        incomplete_last_read_ = false;
    };
    
    while (!is_end(ch)) {
        if (is_newline(ch) && !in_quotes_) {
            add_field(ch);
            consumed_++;
            return ParseStatus::complete;
        }
        
        if (is_delim(ch) && !in_quotes_) {
            add_field(ch);
            field_start = ch+1;
        }
        else if (is_quote(ch)) {
            // double quote => literal
            if (!is_end(ch+1) && is_quote(ch+1)) {
                ch++; // skip one char to make literal
                current_field_quote_literals++;
            }
            // one quote => quoting
            else if (in_quotes_) {
                in_quotes_ = false;

                if (!is_end(ch+1)) {
                    bool is_next_delim = is_delim(ch+1);
                    bool is_next_newline = is_newline(ch+1);

                    // wrong quoting => data after quotes
                    if (!is_next_delim && !is_next_newline) {
                        return ParseStatus::fail;
                    }
                    
                    // for delim or newline as next char
                    add_field(ch);
                    ch += 2;
                    consumed_ += 2;
                    field_start = ch;

                    if (is_next_delim) {
                        continue;
                    }

                    if (is_next_newline) {
                        return ParseStatus::complete;
                    }
                }
                // end, so we need to mark field as incomplete and show that the last char was quote
                // if the first character of the next buffer will be also qyote then we will have just literal
                else {
                    incomplete_with_quote_as_last_char_ = true;
                }
            }
            else if (is_beg(ch) || (!is_beg(ch) && is_delim(ch-1))) {
                in_quotes_ = true;
                field_start = ch+1; // skip open quote in field
            }
            else {
                return ParseStatus::fail;
            }
        }
        // skip every other character 
        ch++;
        consumed_++;
    }

    if (incomplete_with_quote_as_last_char_) ch--;

    add_field(ch);
    incomplete_last_read_ = true;

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
            incomplete_last_read_ = true;
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
    if (incomplete_last_read_) {
        fields_[fields_.size()-1] += *field++;
        incomplete_last_read_ = false;
    }

    while(field != fields.end()) {
        fields_.emplace_back(*field++);
    }
}

void Parser::reset() {
    in_quotes_ = false;
    incomplete_last_read_ = false;
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