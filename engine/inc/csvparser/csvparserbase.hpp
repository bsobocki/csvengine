#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <memory>

#include <csvconfig.hpp>

namespace csv {

enum class ParseStatus {complete, need_more_data, fail};

template <typename FieldType>
class ParserBase {
public:
    explicit ParserBase(const Config& config);
    virtual ~ParserBase() = default;

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] virtual ParseStatus parse(std::string_view buffer) = 0;

    [[nodiscard]] size_t consumed() const noexcept;
    [[nodiscard]] std::string_view err_msg() const noexcept;
    std::vector<FieldType>& fields() noexcept;

    virtual void reset() noexcept;

protected:
    virtual void remove_last_char_from_fields() = 0;

    const Config config_;
    std::string err_msg_;
    std::vector<FieldType> fields_;

    bool pending_cr_ = false;
    bool incomplete_last_read_ = false;
    size_t consumed_ = 0;
};

class ViewParser : public ParserBase<std::string_view> {
public:
    using ParserBase<std::string_view>::ParserBase; // inherit all constructors
    virtual void shift_views(const char* buffer_start) = 0;
};

template <typename FieldType>
struct ParserTypeSelector {
    using type = ParserBase<FieldType>;
};

template<>
struct ParserTypeSelector<std::string_view> {
    using type = ViewParser;
};

template <typename FieldType>
using Parser = typename ParserTypeSelector<FieldType>::type;

std::unique_ptr<Parser<std::string>> make_parser(const Config& config);

}