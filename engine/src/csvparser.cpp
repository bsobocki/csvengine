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
    return naive_parse(buffer);
}

Parser::ParseStatus Parser::csv_quotes_parse(std::string_view buffer) {
    // TODO: implement parsing with quotes
    (void)buffer;
    return Parser::ParseStatus::fail;
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
    field_start_ = 0;
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