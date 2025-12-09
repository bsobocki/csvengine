#include <string>
#include <memory>
#include <fstream>
#include <cstddef>
#include <string_view>

constexpr size_t DEFAULT_CAPACITY = 65536; // 64 KB chunk

// circular buffer - we can reread whole buffer or first K bytes
template <size_t N = DEFAULT_CAPACITY>
class CsvBuffer {
    public:
        explicit CsvBuffer(const std::string_view filename)
            : stream_(std::make_unique<std::ifstream>(std::string(filename), std::ios::binary))
            , data_(std::make_unique<char[]>(N))
        {
            if (!stream_->good()) {
                throw std::runtime_error("Failed to open file: " + std::string(filename));
            }
        }

        // for testing
        explicit CsvBuffer(std::unique_ptr<std::istream> stream)
            : stream_(std::move(stream))
            , data_(std::make_unique<char[]>(N)) {}

        enum class ReadingResult {ok, eof, fail};

        // overwrite only first bytes of the buffer with new data
        ReadingResult read_first_bytes_of_data(size_t bytes) {
            bytes = std::min(bytes, capacity_);

            stream_->read(data_.get(), bytes);

            size_ = stream_->gcount();
            start_ = 0;

            if (size_ == 0)
                return stream_->eof() ? ReadingResult::eof : ReadingResult::fail;

            return ReadingResult::ok;
        }

        // overwrite whole buffer with new data
        ReadingResult read_data() {
            return read_first_bytes_of_data(capacity_);
        }

        size_t available_data_size() const {
            return size_ - start_;
        }

        size_t consumed_data_size() const {
            return start_;
        }

        // for reading char-by-char -- compiler cannot optimize it like range-based for loop on std::string_view
        // so it can be slower than returning chunk-by-chunk and make range-based for loop on chunks
        bool next_char(char& c) {
            if (available_data_size() > 0) {
                c = data_[start_++];
                return true;
            }
            return false;
        }

        // for range-based for loop on available data
        std::string_view consume_bytes(size_t bytes) {
            size_t available = std::min(bytes, available_data_size());
            if (available > 0) {
                auto result = std::string_view(&data_[start_], available);
                start_ += available;
                return result;
            }
            return {};
        }

        // for range-based for loop on available data
        std::string_view consume_available_bytes() {
            return consume_bytes(available_data_size());
        }

        void shift(const size_t bytes) {
            start_ = std::min(start_ + bytes, size_);
        }

        void shift() {
            start_ = size_;
        }

        std::string_view peek(const size_t bytes) const {
            auto available = std::min(available_data_size(), bytes);
            return std::string_view(&data_[start_], available);
        }

        std::string_view peek() const {
            return  std::string_view(&data_[start_], available_data_size());
        }

        size_t position() const {
            return start_;
        }

        bool eof() const {
            return available_data_size() == 0 && stream_->eof();
        }

        bool good() const {
            return stream_->good();
        }

        void reset() {
            stream_->clear();
            stream_->seekg(0);
            start_ = 0;
            size_ = 0;
        }

    private:
        std::unique_ptr<std::istream> stream_;
        std::unique_ptr<char[]> data_;
        size_t size_ = 0;
        size_t start_ = 0;
        const size_t capacity_ = N;
};