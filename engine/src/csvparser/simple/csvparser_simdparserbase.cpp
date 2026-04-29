#include <csvparser/csvparser.hpp>
#include <cstring>

#if defined(__SSE2__)

namespace csv {

template <typename FieldType>
SimdParserBase<FieldType>::SimdParserBase(const Config& config): Parser<FieldType>(config) {
    delim_positions_.reserve(256);
}

template <typename FieldType>
size_t SimdParserBase<FieldType>::scan_for_structural_chars(std::string_view buffer) {
    delim_positions_.clear();

    const char newline = this->config_.line_ending == Config::LineEnding::cr ? '\r' : '\n';
    const char delim = this->config_.delimiter;
    const char* data = buffer.data();
    const size_t size = buffer.size();

    size_t newline_pos = SIZE_MAX;
    size_t i = 0;

    const __m128i vec_newline = _mm_set1_epi8(newline);
    const __m128i vec_delim = _mm_set1_epi8(delim);

    while (i + 16 <= size) {
        __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));

        unsigned mask_newline = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vec_newline)));
        unsigned mask_delim = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vec_delim)));

        if (mask_newline != 0) {
            unsigned newline_bit = __builtin_ctz(mask_newline);
            newline_pos = i + newline_bit;

            mask_delim &= (1u << newline_bit) - 1;
            while (mask_delim) {
                unsigned delim_bit = __builtin_ctz(mask_delim);
                delim_positions_.push_back(static_cast<size_t>(i + delim_bit));
                mask_delim &= mask_delim - 1;
            }
            break;
        }

        while (mask_delim) {
            unsigned delim_bit = __builtin_ctz(mask_delim);
            delim_positions_.push_back(static_cast<size_t>(i + delim_bit));
            mask_delim &= mask_delim - 1;
        }

        i += 16;
    }

    if (newline_pos == SIZE_MAX) {
        for (; i < size; i++) {
            if (data[i] == newline) {
                newline_pos = i;
                break;
            }
            else if (data[i] == delim) {
                delim_positions_.push_back(static_cast<size_t>(i));
            }
        }
    }

    return newline_pos;
}

template <typename FieldType>
void SimdParserBase<FieldType>::extract_and_save_fields(std::string_view buffer, size_t end_pos) {
    const char* data = buffer.data();
    size_t start = 0;
    bool first = true;

    for (size_t delim_pos : delim_positions_) {
        if (delim_pos >= end_pos) break;

        std::string_view field(data + start, delim_pos - start);
        if (first && this->incomplete_last_read_ && this->has_fields()) {
            this->merge_incomplete_field(field);
        }
        else {
            this->add_field(field);
        }
        first = false;
        start = delim_pos + 1;
    }

    std::string_view last(data + start, end_pos - start);
    if (first && this->incomplete_last_read_ && this->has_fields()) {
        this->merge_incomplete_field(last);
    }
    else {
        this->add_field(last);
    }
}

template <typename FieldType>
ParseStatus SimdParserBase<FieldType>::parse(std::string_view buffer) {
    this->consumed_ = 0;

    if (buffer.empty()) return ParseStatus::need_more_data;

    const char newline = this->config_.line_ending == Config::LineEnding::cr ? '\r' : '\n';
    
    size_t newline_pos = scan_for_structural_chars(buffer);

    if (newline_pos == SIZE_MAX) {
        extract_and_save_fields(buffer, buffer.size());

        this->consumed_ = buffer.size();
        this->incomplete_last_read_ = true;

        if (buffer.back() == '\r') {
            this->pending_cr_ = true;
        }

        return ParseStatus::need_more_data;
    }

    size_t line_end = newline_pos;

    if (this->config_.line_ending == Config::LineEnding::crlf) {
        if (line_end > 0 && buffer[line_end - 1] == '\r') {
            line_end--;

            while (!delim_positions_.empty() && delim_positions_.back() >= line_end) {
                delim_positions_.pop_back();
            }
        }
    }

    this->consumed_ = newline_pos + 1;

    if (line_end == 0) {
        if (this->config_.line_ending == Config::LineEnding::crlf && this->pending_cr_) {
            this->remove_last_char_from_fields();
            this->pending_cr_ = false;
        }
        this->incomplete_last_read_ = false;
        return ParseStatus::complete;
    }

    extract_and_save_fields(buffer, line_end);

    this->incomplete_last_read_ = false;
    return ParseStatus::complete;
}

template class SimdParserBase<std::string>;
template class SimdParserBase<std::string_view>;
}

#endif