#pragma once

struct CSVConfig {
    char delimiter = ',';
    bool has_header = true;
    enum class ErrorHandlerMode { strict, lenient };
    ErrorHandlerMode errorHandlerMode = ErrorHandlerMode::strict;
    enum class LineEnding { Auto, LF, CRLF, CR };
};