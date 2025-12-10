#pragma once

namespace csv {

struct Config {
    char delimiter = ',';
    bool has_header = true;
    
    enum class ErrorHandlerMode { strict, lenient };
    ErrorHandlerMode error_handler_mode = ErrorHandlerMode::strict;
    
    enum class LineEnding { Auto, LF, CRLF, CR };
    LineEnding line_ending = LineEnding::Auto;

    bool is_line_ending(char ch) const {
        switch (line_ending) {
            case LineEnding::Auto:
            case LineEnding::CRLF:
                return ch == '\n' || ch == '\r';
            case LineEnding::LF:
                return ch == '\n';
            case LineEnding::CR:
                return ch == '\r';
            default:
                return false;
        }
    }
};

}