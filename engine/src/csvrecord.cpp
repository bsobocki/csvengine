#include <csvrecord.hpp>

CsvRecord::CsvRecord(const std::vector<std::string>& fields, const std::vector<std::string>& headers):
    fields_(fields), headers_(headers) {
}

std::vector<std::string> CsvRecord::fields() const {
    return fields_;
}