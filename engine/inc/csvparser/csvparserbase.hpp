#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <memory>

#include <csvconfig.hpp>

namespace csv {

enum class ParseStatus {complete, need_more_data, fail};

class Parser {
public:
    explicit Parser(const Config& config);
    virtual ~Parser() = default;

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] virtual ParseStatus parse(std::string_view buffer) = 0;

    [[nodiscard]] size_t consumed() const noexcept;
    [[nodiscard]] std::string_view err_msg() const noexcept;
    std::vector<std::string> move_fields() noexcept;
    const std::vector<std::string>& peek_fields() const noexcept;
    const std::vector<std::string_view> fields_view() const noexcept;

    virtual void reset() noexcept;

protected:
    const Config config_;
    std::string err_msg_;
    std::vector<std::string> fields_;

    bool pending_cr_ = false;
    bool incomplete_last_read_ = false;
    size_t consumed_ = 0;
};

std::unique_ptr<Parser> make_parser(const Config& config);

}