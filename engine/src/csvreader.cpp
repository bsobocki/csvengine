#include <csvreader.hpp>
#include <csvparser.hpp>
#include <optional>
#include <format>

using namespace csv;
using namespace std;

Reader::Reader(const std::string& filepath, const Config config)
    : csv_file_path_(filepath), buffer_(filepath), config_(config), parser_(config_) {

    if (!buffer_.good())
        throw std::runtime_error("Reader: cannot open " + filepath);

    if (config_.has_header) {
        read_headers();
    }
}

Reader::Reader(std::unique_ptr<std::istream> stream, const Config config)
    : buffer_(std::move(stream)), config_(config), parser_(config_) {
        
    if (!buffer_.good())
        throw std::runtime_error("Reader: cannot deal with stream ");

    if (config_.has_header) {
        read_headers();
    }
}

void Reader::read_headers() {
    if (!next()) throw std::runtime_error("Cannot read headers!");
    auto headers_view = current_record().fields();
    headers_ = std::vector<std::string>(headers_view.begin(), headers_view.end());
}

[[nodiscard]] bool Reader::next() {
    parser_.reset();

    while (true) {
        if (buffer_.empty()) {
            auto refill_result = buffer_.refill();

            if (refill_result == Buffer<>::ReadingResult::eof) {
                auto fields = parser_.move_fields();

                if (!fields.empty()) {
                    current_record_ = Record(std::move(fields));
                    return true;
                }

                return false;
            }

            if (refill_result != Buffer<>::ReadingResult::ok) {
                return false;
            }
        }

        auto data = buffer_.view();
        auto result = parser_.parse(data);
        buffer_.consume(parser_.consumed());

        if (result == Parser::ParseStatus::complete) {
            current_record_ = Record(parser_.move_fields());
            return true;
        }

        if (result == Parser::ParseStatus::fail) {
            break;
        }
    }

    return false; // on Parser::ParseStatus::fail
}

Reader::Iterator::Iterator(Reader* reader): reader_(reader) {
    if (reader_ && reader_->line_number() == 0) {
        operator++();
    }
}

Reader::Iterator& Reader::Iterator::operator++() {
    if (reader_ && !reader_->next())
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