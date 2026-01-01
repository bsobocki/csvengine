#pragma once

namespace csv {

struct Config {
    char delimiter = ',';
    bool has_header = true;
    bool has_quoting = true;
    char quote_char = '\"';

    enum class ParseMode { strict, lenient };
    ParseMode parse_mode = ParseMode::strict;

    enum class LineEnding { auto, lf, crlf, cr };
    LineEnding line_ending = LineEnding::auto;

    enum class RecordSizePolicy {
        flexible,          // Allow any size (default for compatibility)
        strict_to_first,   // All records must match first record
        strict_to_header,  // All records must match header count
        strict_to_value    // User specifies expected size
    };
    RecordSizePolicy record_size_policy = RecordSizePolicy::strict_to_first;

    bool is_line_ending(char ch) const {
        switch (line_ending) {
            case LineEnding::auto:
            case LineEnding::crlf:
                return ch == '\n' || ch == '\r';
            case LineEnding::lf:
                return ch == '\n';
            case LineEnding::cr:
                return ch == '\r';
            default:
                return false;
        }
    }
};

}