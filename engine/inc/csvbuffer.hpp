#include <cstddef>

constexpr size_t DEFAULT_CAPACITY = 65536; // 64 KB chunk

template <size_t N>
class CsvBuffer {
    public:
        explicit CsvBuffer(const std::string_view filename, size_t capacity = DEFAULT_CAPACITY);

        // read whole buffer at once
        std::string_view chunk() const;
        // reading from buffer
        size_t remain_data() const;

    private:
        void read_data();

        std::ifstream _file;
        const size_t _start;
        const size_t _size;
        const char _data[N];
};