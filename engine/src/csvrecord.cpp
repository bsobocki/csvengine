#include <csvrecord.hpp>

CsvRecord::CsvRecord(const std::vector<std::string>& fields, const std::vector<std::string>& headers):
    fields_(fields), headers_(headers) {
}

CsvRecord::CsvRecord(std::vector<std::string>&& fields, std::vector<std::string>&& headers):
    fields_(fields), headers_(headers) {
}

CsvRecord::CsvRecord(const std::vector<std::string_view>& fields, const std::vector<std::string_view>& headers):
    fields_(std::vector<std::string>(fields.begin(), fields.end())),
    headers_(std::vector<std::string>(headers.begin(), headers.end())) {
}

CsvRecord::CsvRecord(std::vector<std::string_view>&& fields, std::vector<std::string>&& headers):
    fields_(std::vector<std::string>(fields.begin(), fields.end())),
    headers_(std::vector<std::string>(headers.begin(), headers.end())) {
}

const std::vector<std::string>& CsvRecord::fields() const {
    return fields_;
}