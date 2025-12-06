#pragma once

struct CsvConfig {
    char delimiter = ',';
    bool has_header = true;
    enum class ErrorHandlerMode { strict, lenient };
    ErrorHandlerMode error_handler_mode = ErrorHandlerMode::strict;
    enum class LineEnding { Auto, LF, CRLF, CR };
    LineEnding line_ending = LineEnding::Auto;
};