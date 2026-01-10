#include <iostream>
#include <cstring>
#include <csvparser.hpp>
#include <cstring>

namespace csv {

SimpleParser::SimpleParser(const Config& config): Parser(config) {}

std::vector<std::string_view> SimpleParser::split(std::string_view str, const char delim) const {
    std::vector<std::string_view> result;
    const char* str_end = str.data() + str.size();
    const char* start = str.data();
    const char* end = static_cast<const char*>(memchr(str.data(), delim, str.size()));

    while(end) {
        result.emplace_back(start, static_cast<size_t>(end - start));
        start = end + 1;
        end = static_cast<const char*>(memchr(start, delim, str_end - start));
    }

    result.emplace_back(start, static_cast<size_t>(str_end - start));

    return result;
}

ParseStatus SimpleParser::parse(std::string_view buffer) {
    consumed_ = 0;

    const char newline = config_.line_ending == Config::LineEnding::cr ? '\r' : '\n';
    const char *newline_ptr = static_cast<const char*>(memchr(buffer.data(), newline, buffer.size()));
    
    if (!newline_ptr) {
        if (!buffer.empty()) {
            auto fields = split(buffer, config_.delimiter);
            insert_fields(fields);
            consumed_ = buffer.size();
            incomplete_last_read_ = true;
        }
        return ParseStatus::need_more_data;
    }

    const size_t newline_pos = static_cast<size_t>(newline_ptr - buffer.data());
    auto line = buffer.substr(0, newline_pos);

    if (config_.line_ending == Config::LineEnding::crlf) {
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1); // strip CR if present [optional]
        }
    }

    consumed_ = newline_pos + 1;

    if (line.empty()) {
        if (config_.line_ending == Config::LineEnding::crlf && incomplete_last_read_ &&
            !fields_.empty() && !fields_.back().empty() && fields_.back().back() == '\r')
        {
            fields_.back().pop_back();
        }
        incomplete_last_read_ = false;
        return ParseStatus::complete;
    }

    auto fields = split(line, config_.delimiter);
    insert_fields(fields);

    incomplete_last_read_ = false;
    return ParseStatus::complete;
}

void SimpleParser::insert_fields(const std::vector<std::string_view>& fields) {
    auto field = fields.begin();
    if (incomplete_last_read_ && !fields_.empty() && field != fields.end()) {
        fields_[fields_.size()-1] += *field++;
    }
    while(field != fields.end()) {
        fields_.emplace_back(*field++);
    }
}

}