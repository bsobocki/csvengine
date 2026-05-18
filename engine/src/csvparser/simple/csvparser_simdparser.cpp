#include <csvparser/csvparser.hpp>
#include <cstring>

#if defined(__SSE2__)

namespace csv {

SimdParser::SimdParser(const Config& config): SimdParserBase<std::string>(config) {}

void SimdParser::remove_last_char_from_fields() {
    fields_.back().pop_back();
}

void SimdParser::merge_incomplete_field(const std::string_view& field) {
    fields_.back() += field;
}

void SimdParser::add_field(const std::string_view& field) {
    fields_.emplace_back(field);
}

bool SimdParser::has_fields() const {
    return !fields_.empty();
}

}

#endif