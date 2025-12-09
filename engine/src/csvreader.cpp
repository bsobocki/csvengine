#include <csvreader.hpp>
#include <csvparser.hpp>
#include <optional>
#include <format>

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
    if (buffer_.eof() || !buffer_.good()) return false;

    CsvParser parser(config_);

    while (true) {
        auto available_data_size = buffer_.available_data_size();
        if (available_data_size == 0) {
            if (buffer_.read_data() != CsvBuffer<>::ReadingResult::ok) {
                return false;
            }
        }

        auto buffer_data = buffer_.consume_available_bytes();
        auto result = parser.parse(buffer_data);
        
        switch(result) {
            case CsvParser::ParseStatus::record:
                buffer_.shift(parser.consumed());
                current_record_ = CsvRecord(parser.fields());
                return true;
            
            case CsvParser::ParseStatus::eob:
                break;
            
            case CsvParser::ParseStatus::fail:
                return false;
        }
    }
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