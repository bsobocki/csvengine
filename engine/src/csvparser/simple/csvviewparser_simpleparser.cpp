#include <iostream>
#include <csvparser/csvparser.hpp>
#include <cstring>

namespace csv {

ViewSimpleParser::ViewSimpleParser(const Config& config): SimpleParserBase(config) {}

// contract: after ParseStatus::need_more_data methods parse and inser_fields must be called after adjust_fields !
ParseStatus ViewSimpleParser::parse(std::string_view buffer) {
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
            line.remove_suffix(1);
        }
    }

    consumed_ = newline_pos + 1;

    if (line.empty()) {
        if (config_.line_ending == Config::LineEnding::crlf && pending_cr_) {
            fields_.back().remove_suffix(1);
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

// contract: after ParseStatus::need_more_data methods parse and inser_fields must be called after adjust_fields !
void ViewSimpleParser::merge_incomplete_field(const std::string_view& field) {
    auto new_size = fields_.back().size() + field.size();
    fields_.back() = std::string_view(fields_.back().data(), new_size);
}

void ViewSimpleParser::add_field(const std::string_view& field) {
    fields_.push_back(field);
}

void ViewSimpleParser::shift_views(const char* new_buffer_start) {
    if (fields_.empty()) return;

    auto old_fields_data_start = fields_[0].data();

    for (auto& field : fields_) {
        auto offset = field.data() - old_fields_data_start;
        field = std::string_view(new_buffer_start + offset, field.size());
    }
}

const std::vector<std::string_view>& ViewSimpleParser::fields() const noexcept {
    return fields_;
}

void ViewSimpleParser::reset() noexcept {
    Parser::reset();
    fields_.clear();
}

}