#include <csvreader.hpp>
#include <csvparser.hpp>
#include <csverrors.hpp>
#include <csvbuffer/csvstreambuffer.hpp>
#include <csvbuffer/csvmappedbuffer.hpp>
#include <optional>
#include <format>

using namespace std;

namespace csv {

ReaderRecordView::ReaderRecordView(const std::string& filepath, const Config& config)
    : csv_file_path_(filepath)
    , config_(config)
    , parser_(std::make_unique<SimpleParserRecordView>(config))
{
    create_buffer(filepath);
    init();
}

ReaderRecordView::ReaderRecordView(std::unique_ptr<std::istream> stream, const Config& config)
    : buffer_(make_stream_buffer(std::move(stream)))
    , config_(config)
    , parser_(std::make_unique<SimpleParserRecordView>(config))
{
    init();
}

ReaderRecordView::ReaderRecordView(std::unique_ptr<IBuffer> buffer, const Config& config)
    : buffer_(std::move(buffer))
    , config_(config)
    , parser_(std::make_unique<SimpleParserRecordView>(config))
{
    init();
}

void ReaderRecordView::init() {
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

void ReaderRecordView::read_headers() {
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

    current_record_ = RecordView();
    line_number_ = 0;
}

void ReaderRecordView::create_buffer(const std::string& filepath) {
    if (config_.mapped_buffer) {
        buffer_ = make_mapped_buffer(filepath);
    }
    else {
        buffer_ = make_stream_buffer(filepath);
    }
}

bool ReaderRecordView::next() {
    parser_->reset();

    auto save_record = [&, policy = config_.record_size_policy](std::vector<std::string_view>&& fields) {
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
        current_record_ = RecordView(std::move(fields));
        line_number_++;
    };

    bool need_to_compact_data = false;
    size_t consumed = 0;
    while (true) {
        if (buffer_->empty() || need_to_compact_data) {

            if (consumed >= buffer_->capacity()) {
                throw std::runtime_error("Too big field for current buffer size!");
            }

            auto refill_result = buffer_->refill();

            if (refill_result == ReadingResult::eof) {
                auto fields = parser_->fields();
                buffer_->consume(parser_->consumed());

                if (!fields.empty()) {
                    save_record(std::move(fields));
                    return true;
                }
                return false;
            }

            if (refill_result != ReadingResult::ok) {
                buffer_->consume(parser_->consumed());
                return false;
            }

            // in case ParseStatus::need_more_data - adjust fields to the new buffer and then consumed data to move to the next 
            parser_->shift_views(buffer_->view().data());
            buffer_->consume(parser_->consumed());
            need_to_compact_data = false;
        }

        auto data = buffer_->view();
        auto result = parser_->parse(data);
        consumed += parser_->consumed();

        // if we need more data then we need to refill buffer with moving our data to the beggining
        if (result == ParseStatus::need_more_data) {
            need_to_compact_data = true;
            continue;
        }
        else {
            buffer_->consume(parser_->consumed());
        }

        if (result == ParseStatus::complete) {
            auto fields = parser_->fields();
            save_record(std::move(fields));
            return true;
        }

        if (result == ParseStatus::fail) {
            break;
        }
    }

    return false; // on ParseStatus::fail
}

ReaderRecordView::Iterator::Iterator(ReaderRecordView* reader): reader_(reader) {
    if (reader_ && reader_->line_number() == 0) {
        operator++();
    }
}

ReaderRecordView::Iterator& ReaderRecordView::Iterator::operator++() {
    if (reader_ && !reader_->next())
        reader_ = nullptr;

    return *this;
}

const RecordView& ReaderRecordView::Iterator::operator*() const {
    return reader_->current_record();
}

bool ReaderRecordView::Iterator::operator!=(const Iterator& other) const {
    return this->reader_ != other.reader_;
}

ReaderRecordView::Iterator ReaderRecordView::begin() {
    return ReaderRecordView::Iterator(this);
}

ReaderRecordView::Iterator ReaderRecordView::end() {
    return ReaderRecordView::Iterator(nullptr);
}

bool ReaderRecordView::good() const noexcept {
    return buffer_->good();
}

bool ReaderRecordView::has_header() const noexcept {
    return config_.has_header;
}

std::size_t ReaderRecordView::line_number() const noexcept {
    return line_number_;
}

std::size_t ReaderRecordView::record_size() const noexcept {
    return record_size_;
}

size_t ReaderRecordView::expected_record_size(size_t record_size) const noexcept {
    if (config_.record_size_policy == Config::RecordSizePolicy::flexible) {
        return record_size;
    }
    return record_size_;
}

ReaderRecordView::operator bool() const noexcept {
    return good();
}

Config ReaderRecordView::config() const noexcept {
    return config_;
}

void ReaderRecordView::validate_config() const {
    auto policy = config_.record_size_policy;
    if (policy == Config::RecordSizePolicy::strict_to_header && !config_.has_header) {
        throw ConfigError("strict_to_header requires has_header=true");
    }

    if (policy == Config::RecordSizePolicy::strict_to_value && config_.record_size == 0) {
        throw ConfigError("strict_to_value policy requires record_size > 0");
    }
}

const RecordView& ReaderRecordView::current_record() const noexcept {
    return current_record_;
}
    
const std::vector<std::string>& ReaderRecordView::headers() const noexcept {
    return headers_;
}

}