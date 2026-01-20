#include <iostream>
#include <cstring>
#include <csvparser.hpp>

namespace csv {

LenientQuotingParser::LenientQuotingParser(const Config& config): QuotingParser(config) {}

// TODO: CODE REFACTOR (split into functions + extract common logic and code into QuotingParser class)

ParseStatus LenientQuotingParser::parse(std::string_view buffer) {
    consumed_ = 0;
    bool quoted_field = false;

    if (buffer.empty()) return ParseStatus::need_more_data;

    const char* buff_it = buffer.data();
    const char* buff_end = buff_it + buffer.size();

    auto field_start = buff_it;

    const auto is_begin   = [&](auto ptr) { return ptr == buffer.data(); };
    const auto is_end     = [&](auto ptr) { return ptr == buff_end; };
    const auto consume    = [&](size_t consume_size = 1) {
        buff_it += consume_size;
        consumed_ += consume_size;
    };
    const auto add_field = [&](auto end_it) {
        if (!incomplete_last_read_) {
            fields_.emplace_back();
        }

        std::string& field_ref = fields_.back();
        bool quoting = false;

        // after need_more_data we also need to keep info about quoting
        // because in lenient mode the first quote will set in_quotes_ to false
        // but add_field need to know that this field is in quotes
        // even though we don't have quote at the start
        if (quoted_field) {
            quoting = true;
            quoted_field = false;
        }

        if (is_quote(*field_start)) {
            field_start++;
            quoting = true;
        }

        auto raw_len = std::distance(field_start, end_it);
        field_ref.reserve(field_ref.size() + raw_len); // even if we will use slightly less space - we will avoid repeated reallocations

        auto it = field_start;
        while (it != end_it) {
            if (is_quote(*it)) {
                auto next = it + 1;

                if (quoting) {
                    // "" escape - add one quote, skip both
                    if (next != end_it && is_quote(*next)) {
                        field_ref += *it;
                        it += 2;
                        continue;
                    }
                    else {
                        quoting = false;
                        it++;
                        continue;
                    }
                }
            }
            field_ref += *it++;
        }
        incomplete_last_read_ = false;
    };

    if (config_.line_ending == Config::LineEnding::crlf && pending_cr_) {
        pending_cr_ = false;
        if (is_newline(*buff_it)) {
            consume();
            remove_last_saved_char();
            return ParseStatus::complete;
        }
        // in other case just treat \r as data
    }

    if (pending_quote_) {
        pending_quote_ = false;

        if (is_quote(*buff_it)) {
            in_quotes_ = true;
            fields_.back() += config_.quote_char;  // Add the literal quote
            consume();

            if (is_end(buff_it)) {
                incomplete_last_read_ = true;
                return ParseStatus::need_more_data;
            }
            field_start = buff_it;
        }
        else if (is_newline(*buff_it)) {
            consume();
            return ParseStatus::complete;
        }
        else if (is_delim(*buff_it)) {
            consume(); // we already have the field so we can just skip delim to avoid adding new field
            field_start = buff_it;
            incomplete_last_read_ = false;
        }
        else {
            // LENIENT: data after quote = quote was closing, continue as literal
            in_quotes_ = false;
        }
    }

    // we need to let add_field know that even though the first char is not a quote
    // we are still in quotes
    quoted_field = in_quotes_;

    while (!is_end(buff_it)) {
        if (is_newline(*buff_it) && !in_quotes_) {
            auto field_end = buff_it;
            if (config_.line_ending == Config::LineEnding::crlf) {
                if (!is_begin(field_end) && *(field_end-1) == '\r') {
                    field_end--;
                }
            }
            add_field(field_end);
            consume();
            return ParseStatus::complete;
        }
        else if (is_delim(*buff_it) && !in_quotes_) {
            add_field(buff_it);
            field_start = buff_it + 1;
        }
        else if (is_quote(*buff_it)) {
            auto next_buff_it = buff_it + 1;

            if (in_quotes_) {
                // double quotes in quoting means one quote literal                
                if (!is_end(next_buff_it) && is_quote(*next_buff_it)) {
                    consume(2);
                    continue;
                }

                // every single quote cancel in_quotes_
                in_quotes_ = false;

                if (is_end(next_buff_it)) {
                    // Quote at buffer end
                    add_field(buff_it);  // Add field without closing quote
                    pending_quote_ = true;
                    incomplete_last_read_ = true;
                    consume();
                    return ParseStatus::need_more_data;
                }
            }
            else if (buff_it == field_start) {
                in_quotes_ = true;
            }
            // if not at start and not in quotes then it is just a single literal character
        }
        // skip every other character
        consume();
    }

    if (config_.line_ending == Config::LineEnding::crlf && !in_quotes_ &&
            !buffer.empty() && buffer.back() == '\r')
    {
        pending_cr_ = true;
    }

    add_field(buff_it);
    incomplete_last_read_ = true;

    return ParseStatus::need_more_data;
}

}