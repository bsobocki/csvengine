#include <iostream>
#include <csvparser/csvparser.hpp>
#include <cstring>

namespace csv {

SimpleParser::SimpleParser(const Config& config): SimpleParserBase(config) {}

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
            if (buffer.back() == '\r') {
                pending_cr_ = true;
            }
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
        if (config_.line_ending == Config::LineEnding::crlf && pending_cr_) {
            fields_.back().pop_back();
            pending_cr_ = false;
        }
        incomplete_last_read_ = false;
        return ParseStatus::complete;
    }

    auto fields = split(line, config_.delimiter);
    insert_fields(fields);

    incomplete_last_read_ = false;
    return ParseStatus::complete;
}

void SimpleParser::merge_incomplete_field(const std::string_view& field) {
    fields_.back() += field;
}

void SimpleParser::add_field(const std::string_view& field) {
    fields_.emplace_back(field);
}

}