#include <csvparser/csvparser.hpp>
#include <cstring>

namespace csv {

SimpleParserBase::SimpleParserBase(const Config& config): Parser(config) {}

std::vector<std::string_view> SimpleParserBase::split(std::string_view str, const char delim) const {
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

void SimpleParserBase::insert_fields(const std::vector<std::string_view>& fields) {
    auto field = fields.begin();
    if (incomplete_last_read_ && !fields_.empty() && field != fields.end()) {
        merge_incomplete_field(*field++);
    }
    while(field != fields.end()) {
        add_field(*field++);
    }
}

}