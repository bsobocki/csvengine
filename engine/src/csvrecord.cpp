#include <csvrecord.hpp>

using namespace csv;

Record::Record(const std::vector<std::string>& fields, const std::vector<std::string>& headers):
    fields_(fields), headers_(headers) {
}

Record::Record(std::vector<std::string>&& fields, std::vector<std::string>&& headers):
    fields_(fields), headers_(headers) {
}

Record::Record(const std::vector<std::string_view>& fields, const std::vector<std::string_view>& headers):
    fields_(fields.begin(), fields.end()),
    headers_(headers.begin(), headers.end()) {
}

Record::Record(std::vector<std::string_view>&& fields, std::vector<std::string>&& headers):
    fields_(fields.begin(), fields.end()),
    headers_(headers.begin(), headers.end()) {
}

const std::vector<std::string>& Record::fields() const {
    return fields_;
}