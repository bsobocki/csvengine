#include <gmock/gmock.h>
#include <csvbuffer/csvbuffer.hpp>

namespace csv {

class MockBuffer : public IBuffer {
public:
    MOCK_METHOD(ReadingResult, refill, (), (override));
    MOCK_METHOD(std::string_view, view, (), (const, noexcept, override));
    MOCK_METHOD(void, consume, (size_t), (noexcept, override));
    MOCK_METHOD(size_t, available, (), (const, noexcept, override));
    MOCK_METHOD(size_t, capacity, (), (const, noexcept, override));
    MOCK_METHOD(bool, empty, (), (const, noexcept, override));
    MOCK_METHOD(bool, eof, (), (const, noexcept, override));
    MOCK_METHOD(bool, good, (), (const, noexcept, override));
    MOCK_METHOD(bool, reset, (), (override));
};

}