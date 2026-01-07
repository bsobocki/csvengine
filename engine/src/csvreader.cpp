#include <csvreader.hpp>
#include <csvparser.hpp>
#include <csverrors.hpp>
#include <optional>
#include <format>

using namespace std;

namespace csv {

Reader::Reader(const std::string& filepath, const Config& config)
    : csv_file_path_(filepath)
    , buffer_(make_buffer(filepath))
    , config_(config)
    , parser_(make_parser(config))
{
    init();
}

Reader::Reader(std::unique_ptr<std::istream> stream, const Config& config)
    : buffer_(make_buffer(std::move(stream)))
    , config_(config)
    , parser_(make_parser(config))
{
    init();
}

Reader::Reader(std::unique_ptr<IBuffer> buffer, const Config& config)
    : buffer_(std::move(buffer))
    , config_(config)
    , parser_(make_parser(config))
{
    init();
}

void Reader::init() {
    validate_config();

    if (config_.record_size_policy == Config::RecordSizePolicy::strict_to_value) {
        record_size_ = config_.record_size;
    }

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

    headers_ = std::vector<std::string>(
        current_record_.fields().begin(),
        current_record_.fields().end()
    );

    if (config_.record_size_policy == Config::RecordSizePolicy::strict_to_header) {
        record_size_ = headers_.size();
    }

    current_record_ = Record();
    line_number_ = 0;
}

bool Reader::next() {
    parser_->reset();

    auto save_record = [&, policy = config_.record_size_policy](std::vector<std::string>&& fields) {
        if (record_size_ == 0) {
            if (policy == Config::RecordSizePolicy::strict_to_first)
            {
                record_size_ = fields.size();
            }
        }
        else {
            auto expected_size = expected_record_size(fields.size());
            if (expected_size != fields.size()) {
                throw RecordSizeError(line_number_, expected_size, fields.size());
            }
        }
        current_record_ = Record(std::move(fields));
        line_number_++;
    };

    while (true) {
        if (buffer_->empty()) {
            auto refill_result = buffer_->refill();

            if (refill_result == ReadingResult::eof) {
                auto fields = parser_->move_fields();

                if (!fields.empty()) {
                    save_record(std::move(fields));
                    return true;
                }
                return false;
            }

            if (refill_result != ReadingResult::ok) {
                return false;
            }
        }

        auto data = buffer_->view();
        auto result = parser_->parse(data);
        buffer_->consume(parser_->consumed());

        if (result == ParseStatus::complete) {
            save_record(parser_->move_fields());
            return true;
        }

        if (result == ParseStatus::fail) {
            break;
        }
    }

    return false; // on ParseStatus::fail
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

bool Reader::good() const noexcept {
    return buffer_->good();
}

bool Reader::has_header() const noexcept {
    return config_.has_header;
}

std::size_t Reader::line_number() const noexcept {
    return line_number_;
}

std::size_t Reader::record_size() const noexcept {
    return record_size_;
}

size_t Reader::expected_record_size(size_t record_size) const noexcept {
    if (config_.record_size_policy == Config::RecordSizePolicy::flexible) {
        return record_size;
    }
    return record_size_;
}

Reader::operator bool() const noexcept {
    return good();
}

Config Reader::config() const noexcept {
    return config_;
}

void Reader::validate_config() const {
    auto policy = config_.record_size_policy;
    if (policy == Config::RecordSizePolicy::strict_to_header && !config_.has_header) {
        throw ConfigError("strict_to_header requires has_header=true");
    }

    if (policy == Config::RecordSizePolicy::strict_to_value && config_.record_size == 0) {
        throw ConfigError("strict_to_value policy requires record_size > 0");
    }
}

const Record& Reader::current_record() const noexcept {
    return current_record_;
}
    
const std::vector<std::string>& Reader::headers() const noexcept {
    return headers_;
}

}