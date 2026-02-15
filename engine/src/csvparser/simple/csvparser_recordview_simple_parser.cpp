#include <iostream>
#include <csvparser.hpp>
#include <cstring>

namespace csv {

ViewSimpleParser::ViewSimpleParser(const Config& config): Parser(config) {}

std::vector<std::string_view> ViewSimpleParser::split(std::string_view str, const char delim) const {
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
void ViewSimpleParser::insert_fields(const std::vector<std::string_view>& fields) {
    auto field = fields.begin();
    if (incomplete_last_read_ && !fields_.empty() && field != fields.end()) {
        auto new_size = fields_[fields_.size()-1].size() + (*field).size();
        fields_.back() = std::string_view(fields_[fields_.size()-1].data(), new_size);
        field++;
    }
    while(field != fields.end()) {
        fields_.push_back(*field++);
    }
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