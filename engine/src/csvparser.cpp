#include <iostream>
#include <cstring>
#include <csvparser.hpp>

using namespace csv;

namespace {

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

    if (buffer.empty()) return ParseStatus::need_more_data;

    auto buff_it = buffer.begin();
    auto field_start = buff_it;
    size_t current_field_quote_literals = 0;    

    const auto is_end     = [end = buffer.end()](auto it) { return it == end; };
    const auto is_quote   = [this](char c)  { return c == config_.quote_char; };
    const auto is_delim   = [this](char c)  { return c == config_.delimiter; };
    const auto is_newline = [this](char c)  { return config_.is_line_ending(c); };
    const auto consume    = [&](size_t consume_size = 1) {
        buff_it += consume_size;
        consumed_ += consume_size;
    };
    const auto add_field =  [&](auto end_it) {
        if (!incomplete_last_read_) {
            fields_.emplace_back();
        }

        std::string& field_ref = fields_.back();
        size_t write_start = field_ref.size();

        size_t curr_field_size = std::distance(field_start, end_it) - current_field_quote_literals;
        field_ref.resize(field_ref.size() + curr_field_size);

        if (current_field_quote_literals == 0) {
            // explicit char* converion by &*
            std::memcpy(field_ref.data() + write_start, &*field_start, curr_field_size);
        }
        else {
            auto it = field_start;
            while (it != end_it) {
                field_ref[write_start++] = *it++;
                // for double quotes just skip the next one
                if (it != end_it && is_quote(*it) && is_quote(*(it-1))) it++;
            }
        }
        
        current_field_quote_literals = 0;
        incomplete_last_read_ = false;
    };

    if (pending_quote_) {
        pending_quote_ = false;

        if (is_quote(*buff_it)) {
            in_quotes_ = true;
            consume();

            if (is_end(buff_it)) {
                incomplete_last_read_ = true;
                return ParseStatus::need_more_data;
            }
        }
        else if (!is_delim(*buff_it) && !is_newline(*buff_it)) {
            return ParseStatus::fail;
        }
        else if (is_newline(*buff_it)) {
            consume();
            return ParseStatus::complete;
        }
        else if (is_delim(*buff_it)) {
            consume(); // we already have the field so we can just skip delim to avoid adding new field
            field_start = buff_it;
            incomplete_last_read_ = false;
        }
    }
    
    while (!is_end(buff_it)) {
        if (is_newline(*buff_it) && !in_quotes_) {
            add_field(buff_it);
            consume();
            return ParseStatus::complete;
        }
        
        if (is_delim(*buff_it) && !in_quotes_) {
            add_field(buff_it);
            consume();
            field_start = buff_it;
            continue;
        }
        
        if (is_quote(*buff_it)) {
            auto next_buff_it = buff_it + 1;

            // double quote => literal
            if (in_quotes_ && !is_end(next_buff_it) && is_quote(*next_buff_it)) {
                consume(); // skip one char to make literal
                current_field_quote_literals++;
            }
            // one quote => quoting
            else if (in_quotes_) {
                in_quotes_ = false;

                if (!is_end(next_buff_it)) {
                    bool is_next_delim = is_delim(*next_buff_it);
                    bool is_next_newline = is_newline(*next_buff_it);

                    // wrong quoting => data after quotes
                    if (!is_next_delim && !is_next_newline) {
                        return ParseStatus::fail;
                    }
                    
                    // for delim or newline as next char
                    add_field(buff_it);
                    consume(2);
                    field_start = buff_it;

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
                    // Quote at buffer end
                    add_field(buff_it);  // Add field without closing quote
                    pending_quote_ = true;
                    incomplete_last_read_ = true;
                    consume();
                    return ParseStatus::need_more_data;
                }
            }
            else if (buff_it == field_start) {
                in_quotes_ = true;
                field_start = buff_it + 1; // skip open quote in field
            }
            else {
                return ParseStatus::fail;
            }
        }
        // skip every other character 
        consume();
    }

    add_field(buff_it);
    incomplete_last_read_ = true;

    return Parser::ParseStatus::need_more_data;
}

// lenient mode
Parser::ParseStatus Parser::csv_quotes_lenient_parse(std::string_view buffer) {
    // TODO: implement lenient parse and replace it
    return csv_quotes_strict_parse(buffer);
}

// naive implementation without quotes
Parser::ParseStatus Parser::naive_parse(std::string_view buffer) {
    consumed_ = 0;

    auto newline_pos = buffer.find('\n');

    if (newline_pos == std::string_view::npos) {
        if (buffer.empty()) {
            incomplete_last_read_ = true;
            return Parser::ParseStatus::need_more_data;
        }

        auto fields = split(buffer, config_.delimiter);
        insert_fields(fields);
        consumed_ = buffer.size();
        
        return Parser::ParseStatus::need_more_data;
    }

    auto line = buffer.substr(0, newline_pos);

    if (line.empty()) {
        consumed_ = 1;
        return Parser::ParseStatus::complete;
    }

    auto fields = split(line, config_.delimiter);
    insert_fields(fields);
    consumed_ += line.size()  + 1; // + newline character

    return Parser::ParseStatus::complete;
}

// for naive implementation
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
    pending_quote_= false;
    incomplete_last_read_ = false;
    fields_.clear();
    consumed_ = 0;
    err_msg_.clear();
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