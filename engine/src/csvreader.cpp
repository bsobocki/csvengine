#include <csvreader.hpp>
#include <csvparser.hpp>
#include <csverrors.hpp>
#include <optional>
#include <format>

using namespace csv;
using namespace std;

Reader::Reader(const std::string& filepath, const Config config)
    : csv_file_path_(filepath)
    , buffer_(make_buffer(filepath))
    , config_(config)
    , parser_(config_)
{
    init();
}

Reader::Reader(std::unique_ptr<std::istream> stream, const Config config)
    : buffer_(make_buffer(std::move(stream)))
    , config_(config)
    , parser_(config_)
{
    init();
}

Reader::Reader(std::unique_ptr<IBuffer> buffer, const Config config)
    : buffer_(std::move(buffer))
    , config_(config)
    , parser_(config_)
{
    init();
}

void Reader::init() {
    if (!buffer_->good()) {
        throw BufferError();
    }

    if (config_.has_header) {
        read_headers();
    }
}

void Reader::read_headers() {
    if (!next()) {
        throw FileHeaderError();
    }

    auto headers_view = current_record().fields();
    record_size_ = headers_view.size();
    headers_ = std::vector<std::string>(headers_view.begin(), headers_view.end());
}

bool Reader::next() {
    parser_.reset();

    while (true) {
        if (buffer_->empty()) {
            auto refill_result = buffer_->refill();

            if (refill_result == ReadingResult::eof) {
                auto fields = parser_.move_fields();

                if (!fields.empty()) {
                    current_record_ = Record(std::move(fields));
                    if (record_size_ == 0) {
                        record_size_ = current_record_.fields().size();
                    }
                    return true;
                }
                return false;
            }

            if (refill_result != ReadingResult::ok) {
                return false;
            }
        }

        auto data = buffer_->view();
        auto result = parser_.parse(data);
        buffer_->consume(parser_.consumed());

        if (result == Parser::ParseStatus::complete) {
            current_record_ = Record(parser_.move_fields());
            if (record_size_ == 0) {
                record_size_ = current_record_.fields().size();
            }
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
    return buffer_->good();
}

bool Reader::has_header() const {
    return config_.has_header;
}

std::size_t Reader::line_number () const {
    return current_record_idx_ + 1;
}

std::size_t Reader::record_size() const {
    return record_size_;
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