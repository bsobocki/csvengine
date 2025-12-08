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
            : file_(std::string(filename), std::ios::binary)
            , data_(std::make_unique<char[]>(N)) {}
        
        ~CsvBuffer() {
            file_.close();
        }

        enum class ReadingResult {ok, eof, fail};

        // overwrite only first bytes of the buffer with new data
        ReadingResult read_first_bytes_of_data(size_t bytes) {
            bytes = std::min(bytes, capacity_);

            file_.read(data_.get(), bytes);

            size_ = file_.gcount();
            start_ = 0;

            if (size_ == 0)
                return file_.eof() ? ReadingResult::eof : ReadingResult::fail;

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
        std::string_view consume_all() {
            return consume_bytes(available_data_size());
        }

        void consume(const size_t bytes) {
            start_ = std::min(start_ + bytes, size_);
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
            return available_data_size() == 0 && file_.eof();
        }

        bool is_open() const {
            return file_.is_open();
        }

        bool good() const {
            return file_.good();
        }

        void reset() {
            file_.clear();
            file_.seekg(0);
            start_ = 0;
            size_ = 0;
        }

    private:
        std::ifstream file_;
        size_t size_ = 0;
        size_t start_ = 0;
        std::unique_ptr<char[]> data_;
        const size_t capacity_ = N;
};