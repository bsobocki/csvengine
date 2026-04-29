#pragma once

#include "csvparserbase.hpp"
#include <vector>

#if defined(__SSE2__)

#include <emmintrin.h>

namespace csv {

template <typename FieldType>
class SimdParserBase : public Parser<FieldType> {
public:
    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;

protected:
    explicit SimdParserBase(const Config& config);
    virtual bool has_fields() const = 0;

    virtual void merge_incomplete_field(const std::string_view& field) = 0;
    virtual void add_field(const std::string_view& field) = 0;

private:
    std::vector<size_t> delim_positions_;

    size_t scan_for_structural_chars(std::string_view buffer);
    void extract_and_save_fields(std::string_view buffer, size_t end_pos);
};

class SimdParser : public SimdParserBase<std::string> {
public:
    explicit SimdParser(const Config& config);

private:
    void merge_incomplete_field(const std::string_view& field) override;
    void add_field(const std::string_view& field) override;
    void remove_last_char_from_fields() override;
    bool has_fields() const override;
};


class ViewSimdParser : public SimdParserBase<std::string_view> {
public:
    explicit ViewSimdParser(const Config& config);

    void shift_views(const char* buffer_start);
    bool has_fields() const override;
    void reset() noexcept override;

private:
    void merge_incomplete_field(const std::string_view& field) override;
    void add_field(const std::string_view& field) override;
    void remove_last_char_from_fields() override;
};

}

#endif // defined(__SSE2__)