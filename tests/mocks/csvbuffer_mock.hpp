#include <gmock/gmock.h>
#include <csvbuffer/csvbuffer.hpp>

namespace csv {

class MockBuffer : public IBuffer {
public:
    MOCK_METHOD(ReadingResult, refill, (), (override));
    MOCK_METHOD(std::string_view, view, (), (const, override));
    MOCK_METHOD(void, consume, (size_t), (override));
    MOCK_METHOD(size_t, available, (), (const, override));
    MOCK_METHOD(bool, empty, (), (const, override));
    MOCK_METHOD(bool, eof, (), (const, override));
    MOCK_METHOD(bool, good, (), (const, override));
    MOCK_METHOD(bool, reset, (), (override));
};

}