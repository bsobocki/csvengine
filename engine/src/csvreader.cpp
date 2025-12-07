#include <csvreader.hpp>
#include <optional>

namespace {
    std::vector<std::string_view> split(std::string_view str, const char delim) {
        std::vector<std::string_view> result;

        size_t start = 0;
        size_t end = str.find(delim);

        while(end != std::string_view::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delim, start);
        }

        result.push_back(str.substr(start));

        return result;
    }
}

CsvReader::CsvReader(const std::string& filePath, const CsvConfig config): config_(config) {
    csv_file_.open(filePath, std::ifstream::in);

    if (!csv_file_.good())
        throw std::runtime_error("CsvReader: cannot open " + filePath);

    if (config_.has_header) {
        read_headers();
    }
}

void CsvReader::read_headers() {
    if (!read_next_record()) throw std::runtime_error("Cannot read headers!");
    auto headers_view = current_record().fields();
    headers_ = std::vector<std::string>(headers_view.begin(), headers_view.end());
}

bool CsvReader::read_next_record() {
    // first attempt - naive implementation without quoting
    if (csv_file_.good()) {
        std::string line;
        if (std::getline(csv_file_, line)) {
            auto fields = split(line, config_.delimiter);
            current_record_ = CsvRecord(std::move(fields));
            return true;
        }
    }
    return false;
}

CsvReader::CsvIterator::CsvIterator(CsvReader* reader): reader_(reader) {
    if (reader_ && reader_->line_number() == 0) {
        operator++();
    }
}

CsvReader::CsvIterator& CsvReader::CsvIterator::operator++() {
    if (reader_ && !reader_->read_next_record())
        reader_ = nullptr;

    return *this;
}

const CsvRecord& CsvReader::CsvIterator::operator*() const {
    return reader_->current_record();
}

bool CsvReader::CsvIterator::operator!=(const CsvIterator& other) const {
    return this->reader_ != other.reader_;
}

CsvReader::CsvIterator CsvReader::begin() {
    return CsvReader::CsvIterator(this);
}

CsvReader::CsvIterator CsvReader::end() {
    return CsvReader::CsvIterator(nullptr);
}

bool CsvReader::good() const {
    return csv_file_.good();
}

bool CsvReader::has_header() const {
    return config_.has_header;
}

std::size_t CsvReader::line_number () const {
    return current_record_idx_ + 1;
}

CsvReader::operator bool() const {
    return good();
}

CsvConfig CsvReader::config() const {
    return config_;
}

const CsvRecord& CsvReader::current_record() const {
    return current_record_;
}
    
const std::vector<std::string>& CsvReader::headers() const {
    return headers_;
}