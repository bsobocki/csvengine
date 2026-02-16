#include <iostream>
#include <csvparser/csvparser.hpp>
#include <cstring>

namespace csv {

ViewSimpleParser::ViewSimpleParser(const Config& config): SimpleParserBase(config) {}

void ViewSimpleParser::remove_last_char_from_fields() {
    fields_.back().remove_suffix(1);
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

bool ViewSimpleParser::has_fields() const {
    return !fields_.empty();
}

const std::vector<std::string_view>& ViewSimpleParser::fields() const noexcept {
    return fields_;
}

void ViewSimpleParser::reset() noexcept {
    Parser::reset();
    fields_.clear();
}

}