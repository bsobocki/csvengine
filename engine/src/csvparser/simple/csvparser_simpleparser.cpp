#include <iostream>
#include <csvparser/csvparser.hpp>
#include <cstring>

namespace csv {

SimpleParser::SimpleParser(const Config& config): SimpleParserBase<std::string>(config) {}

void SimpleParser::remove_last_char_from_fields() {
    fields_.back().pop_back();
}

void SimpleParser::merge_incomplete_field(const std::string_view& field) {
    fields_.back() += field;
}

void SimpleParser::add_field(const std::string_view& field) {
    fields_.emplace_back(field);
}

bool SimpleParser::has_fields() const {
    return !fields_.empty();
}

}