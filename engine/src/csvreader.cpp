#include <csvreader.hpp>
#include <csvparser.hpp>
#include <optional>
#include <format>

using namespace csv;
using namespace std;

Reader::Reader(const std::string& filepath, const Config config)
    : csv_file_path_(filepath), buffer_(filepath), config_(config) {

    if (!buffer_.good())
        throw std::runtime_error("Reader: cannot open " + filepath);

    if (config_.has_header) {
        read_headers();
    }
}

Reader::Reader(std::unique_ptr<std::istream> stream, const Config config)
    : buffer_(std::move(stream)), config_(config) {
        
    if (!buffer_.good())
        throw std::runtime_error("Reader: cannot deal with stream ");

    if (config_.has_header) {
        read_headers();
    }
}

void Reader::read_headers() {
    if (!read_next_record()) throw std::runtime_error("Cannot read headers!");
    auto headers_view = current_record().fields();
    headers_ = std::vector<std::string>(headers_view.begin(), headers_view.end());
}

bool Reader::read_next_record() {
    if (buffer_.eof() || !buffer_.good()) return false;

    Parser parser(config_);

    std::string mem;

    while (true) {
        auto available = buffer_.available();
        if (available == 0) {
            if (buffer_.refill() != Buffer<>::ReadingResult::ok) {
                return false;
            }
        }

        auto buffer_data = buffer_.view();
        auto result = parser.parse(buffer_data);
        buffer_.consume(parser.consumed());

        switch(result) {
            case Parser::ParseStatus::record:
                current_record_ = Record(parser.fields());
                return true;

            case Parser::ParseStatus::need_more_data:
                if (buffer_.refill() != Buffer<>::ReadingResult::ok)
                    return false;

                break;
            
            case Parser::ParseStatus::fail:
                return false;
        }
    }
    return true;
}

Reader::Iterator::Iterator(Reader* reader): reader_(reader) {
    if (reader_ && reader_->line_number() == 0) {
        operator++();
    }
}

Reader::Iterator& Reader::Iterator::operator++() {
    if (reader_ && !reader_->read_next_record())
        reader_ = nullptr;

    return *this;
}

const Record& Reader::Iterator::operator*() const {
    return reader_->current_record();
}

bool Reader::Iterator::operator!=(const Iterator& other) const {
    return this->reader_ != other.reader_;
}

Reader::Iterator Reader::begin() {
    return Reader::Iterator(this);
}

Reader::Iterator Reader::end() {
    return Reader::Iterator(nullptr);
}

bool Reader::good() const {
    return buffer_.good();
}

bool Reader::has_header() const {
    return config_.has_header;
}

std::size_t Reader::line_number () const {
    return current_record_idx_ + 1;
}

Reader::operator bool() const {
    return good();
}

Config Reader::config() const {
    return config_;
}

const Record& Reader::current_record() const {
    return current_record_;
}
    
const std::vector<std::string>& Reader::headers() const {
    return headers_;
}