#include <csvparser/csvparser.hpp>
#include <cstring>

namespace csv {

template <typename FieldType>
SimpleParserBase<FieldType>::SimpleParserBase(const Config& config): Parser<FieldType>(config) {}

template <typename FieldType>
std::vector<std::string_view> SimpleParserBase<FieldType>::split(std::string_view str, const char delim) const {
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

template <typename FieldType>
void SimpleParserBase<FieldType>::insert_fields(const std::vector<std::string_view>& fields) {
    auto field = fields.begin();
    if (this->incomplete_last_read_ && has_fields() && field != fields.end()) {
        merge_incomplete_field(*field++);
    }
    while(field != fields.end()) {
        add_field(*field++);
    }
}

template <typename FieldType>
ParseStatus SimpleParserBase<FieldType>::parse(std::string_view buffer) {
    this->consumed_ = 0;

    const char newline = this->config_.line_ending == Config::LineEnding::cr ? '\r' : '\n';
    const char *newline_ptr = static_cast<const char*>(memchr(buffer.data(), newline, buffer.size()));
    
    if (!newline_ptr) {
        if (!buffer.empty()) {
            auto fields = split(buffer, this->config_.delimiter);
            insert_fields(fields);
            this->consumed_ = buffer.size();
            this->incomplete_last_read_ = true;
            if (buffer.back() == '\r') {
                this->pending_cr_ = true;
            }
        }
        return ParseStatus::need_more_data;
    }

    const size_t newline_pos = static_cast<size_t>(newline_ptr - buffer.data());
    auto line = buffer.substr(0, newline_pos);

    if (this->config_.line_ending == Config::LineEnding::crlf) {
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
    }

    this->consumed_ = newline_pos + 1;

    if (line.empty()) {
        if (this->config_.line_ending == Config::LineEnding::crlf && this->pending_cr_) {
            this->remove_last_char_from_fields();
            this->pending_cr_ = false;
        }
        this->incomplete_last_read_ = false;
        return ParseStatus::complete;
    }

    auto fields = split(line, this->config_.delimiter);
    insert_fields(fields);

    this->incomplete_last_read_ = false;
    return ParseStatus::complete;
}

template class SimpleParserBase<std::string>;
template class SimpleParserBase<std::string_view>;
}