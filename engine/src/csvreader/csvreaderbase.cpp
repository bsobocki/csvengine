#include <csvreader/csvreader.hpp>
#include <csvparser/csvparser.hpp>
#include <csverrors.hpp>
#include <csvbuffer/csvstreambuffer.hpp>
#include <csvbuffer/csvmappedbuffer.hpp>
#include <optional>
#include <format>

namespace csv {

template <typename RecordType>
ReaderBase<RecordType>::ReaderBase(const std::string& filepath, const Config& config)
    : csv_file_path_(filepath)
    , config_(config)
{
    create_buffer(filepath);
}

template <typename RecordType>
ReaderBase<RecordType>::ReaderBase(std::unique_ptr<std::istream> stream, const Config& config)
    : buffer_(make_stream_buffer(std::move(stream)))
    , config_(config)
{
}

template <typename RecordType>
ReaderBase<RecordType>::ReaderBase(std::unique_ptr<IBuffer> buffer, const Config& config)
    : buffer_(std::move(buffer))
    , config_(config)
{
}

template <typename RecordType>
void ReaderBase<RecordType>::init() {
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

template <typename RecordType>
void ReaderBase<RecordType>::read_headers() {
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

    current_record_ = RecordType();
    line_number_ = 0;
}

template <typename RecordType>
void ReaderBase<RecordType>::create_buffer(const std::string& filepath) {
    if (config_.mapped_buffer) {
        buffer_ = make_mapped_buffer(filepath);
    }
    else {
        buffer_ = make_stream_buffer(filepath);
    }
}

template <typename RecordType>
ReaderBase<RecordType>::Iterator::Iterator(ReaderBase<RecordType>* reader): reader_(reader) {
    if (reader_ && reader_->line_number() == 0) {
        operator++();
    }
}

template <typename RecordType>
ReaderBase<RecordType>::Iterator& ReaderBase<RecordType>::Iterator::operator++() {
    if (reader_ && !reader_->next())
        reader_ = nullptr;

    return *this;
}

template <typename RecordType>
const RecordType& ReaderBase<RecordType>::Iterator::operator*() const {
    return reader_->current_record();
}

template <typename RecordType>
bool ReaderBase<RecordType>::Iterator::operator!=(const Iterator& other) const {
    return this->reader_ != other.reader_;
}

template <typename RecordType>
ReaderBase<RecordType>::Iterator ReaderBase<RecordType>::begin() {
    return ReaderBase<RecordType>::Iterator(this);
}

template <typename RecordType>
ReaderBase<RecordType>::Iterator ReaderBase<RecordType>::end() {
    return ReaderBase<RecordType>::Iterator(nullptr);
}

template <typename RecordType>
bool ReaderBase<RecordType>::good() const noexcept {
    return buffer_->good();
}

template <typename RecordType>
bool ReaderBase<RecordType>::has_header() const noexcept {
    return config_.has_header;
}

template <typename RecordType>
std::size_t ReaderBase<RecordType>::line_number() const noexcept {
    return line_number_;
}

template <typename RecordType>
std::size_t ReaderBase<RecordType>::record_size() const noexcept {
    return record_size_;
}

template <typename RecordType>
size_t ReaderBase<RecordType>::expected_record_size(size_t record_size) const noexcept {
    if (config_.record_size_policy == Config::RecordSizePolicy::flexible) {
        return record_size;
    }
    return record_size_;
}

template <typename RecordType>
ReaderBase<RecordType>::operator bool() const noexcept {
    return good();
}

template <typename RecordType>
Config ReaderBase<RecordType>::config() const noexcept {
    return config_;
}

template <typename RecordType>
void ReaderBase<RecordType>::validate_config() const {
    auto policy = config_.record_size_policy;
    if (policy == Config::RecordSizePolicy::strict_to_header && !config_.has_header) {
        throw ConfigError("strict_to_header requires has_header=true");
    }

    if (policy == Config::RecordSizePolicy::strict_to_value && config_.record_size == 0) {
        throw ConfigError("strict_to_value policy requires record_size > 0");
    }
}

template <typename RecordType>
const RecordType& ReaderBase<RecordType>::current_record() const noexcept {
    return current_record_;
}

template <typename RecordType>
const std::vector<std::string>& ReaderBase<RecordType>::headers() const noexcept {
    return headers_;
}

template class ReaderBase<Record>;
template class ReaderBase<RecordView>;
}