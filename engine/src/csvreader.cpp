#include <csvreader.hpp>
#include <csvparser.hpp>
#include <optional>
#include <format>

using namespace csv;
using namespace std;

Reader::Reader(const std::string& filepath, const Config config)
    : config_(config), csv_file_path_(filepath), buffer_(filepath) {

    if (!buffer_.good())
        throw std::runtime_error("Reader: cannot open " + filepath);

    std::cout << "CONFIG HAS HEADER? " << config_.has_header << std::endl;
    if (config_.has_header) {
        read_headers();
    }
}

Reader::Reader(std::unique_ptr<std::istream> stream, const Config config)
    : config_(config), buffer_(std::move(stream)) {
        
    if (!buffer_.good())
        throw std::runtime_error("Reader: cannot deal with stream ");

    std::cout << "CONFIG HAS HEADER? " << config_.has_header << std::endl;
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

    // we can go with mem data (string_view) and buffer data (string_view x 2: left and right)
    // we can pass them to parser in order:
    // 1. read mem from start to end
    // 2. if you need more data then read right from start to end
    // 3. if you need more data then read left from start to end
    //
    // this is because for the first buffer read it will read whole buffer (as right)
    // after consuming some data from left we will have two parts: left - used data, right - not used yet
    // at the next read_next_record() call we will use this buffer and there still be two part, but left will be bigger than before
    // at the next read_next_record() we will use right data and gets eob from parser, so we need to read data to the buffer,
    // but we use string_view, so we will need to move the data... but.... maybe buffer_view ???
    // just do char get(size_t index) { return data[(head + index) % size]; } or something
    std::string mem;
    size_t buff_start = 0;
    size_t buff_end = 0;


    while (true) {
        auto available_data_size = buffer_.available_data_size();
        if (available_data_size == 0) {
            if (buffer_.read_data() != Buffer<>::ReadingResult::ok) {
                return false;
            }
        }

        auto buffer_data = buffer_.peek();
        auto result = parser.parse(buffer_data);
        buffer_.shift(parser.consumed());

        switch(result) {
            case Parser::ParseStatus::record:
                current_record_ = Record(parser.fields());
                return true;
            
            case Parser::ParseStatus::eob:
                if (buffer_.read_first_bytes_of_data(buffer_.available_data_size()) != Buffer<>::ReadingResult::ok)
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