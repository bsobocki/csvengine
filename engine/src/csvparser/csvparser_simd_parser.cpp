#include <iostream>
#include <csvparser.hpp>
#include <cstring>

namespace csv {

SimdParser::SimdParser(const Config& config): Parser(config) {}

ParseStatus SimdParser::parse(std::string_view buffer) {
    consumed_ = 0;

    const char newline = config_.line_ending == Config::LineEnding::cr ? '\r' : '\n';
    const char *newline_ptr = static_cast<const char*>(memchr(buffer.data(), newline, buffer.size()));


    incomplete_last_read_ = false;
    return ParseStatus::complete;
}

}