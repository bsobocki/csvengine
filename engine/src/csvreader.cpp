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

CsvReader::CsvReader(const std::string& filepath, const CsvConfig config)
    : config_(config), csv_file_path_(filepath), buffer_(filepath) {

    if (!buffer_.good())
        throw std::runtime_error("CsvReader: cannot open " + filepath);

    std::cout << "CONFIG HAS HEADER? " << config_.has_header << std::endl;
    if (config_.has_header) {
        read_headers();
    }
}

CsvReader::CsvReader(std::unique_ptr<std::istream> stream, const CsvConfig config)
    : config_(config), buffer_(std::move(stream)) {
        
    if (!buffer_.good())
        throw std::runtime_error("CsvReader: cannot deal with stream ");

    std::cout << "CONFIG HAS HEADER? " << config_.has_header << std::endl;
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
    if (!buffer_.good()) return false;

    auto available_data_size = buffer_.available_data_size();
    if (available_data_size == 0) {
        if (buffer_.read_data() != CsvBuffer<>::ReadingResult::ok) {
            return false;
        }
    }

    auto data = buffer_.consume_available_bytes();
    auto newline_pos = data.find('\n');

    if (newline_pos == std::string_view::npos) {
        if (data.empty()) return false;

        auto fields = split(data, config_.delimiter);
        current_record_ = CsvRecord(std::move(fields));
        return true;
    }

    auto line = data.substr(0, newline_pos);

    if (line.empty()) {
        current_record_ = CsvRecord();
        return true;
    }

    auto fields = split(line, config_.delimiter);
    std::cout << "FIELDS: " << fields.size() << std::endl;
    current_record_ = CsvRecord(std::move(fields));
    return true;
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
    return buffer_.good();
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