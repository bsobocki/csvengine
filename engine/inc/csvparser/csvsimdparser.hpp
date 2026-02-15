#pragma once

#include "csvparserbase.hpp"

namespace csv {

class SimdParser : public Parser {
public:
    explicit SimdParser(const Config& config);

    /// @brief function parse doesn't reset the parser state
    [[nodiscard]] ParseStatus parse(std::string_view buffer) override;
};

}