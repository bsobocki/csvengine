#include <csvreader/csvreader.hpp>
#include <csvparser/csvparser.hpp>
#include <csverrors.hpp>
#include <csvbuffer/csvstreambuffer.hpp>
#include <csvbuffer/csvmappedbuffer.hpp>
#include <optional>
#include <format>

namespace csv {

Reader::Reader(const std::string& filepath, const Config& config)
    : ReaderBase<Record>(filepath, config)
    , parser_(make_parser(config))
{
    init();
}

Reader::Reader(std::unique_ptr<std::istream> stream, const Config& config)
    : ReaderBase<Record>(std::move(stream), config)
    , parser_(make_parser(config))
{
    init();
}

Reader::Reader(std::unique_ptr<IBuffer> buffer, const Config& config)
    : ReaderBase<Record>(std::move(buffer), config)
    , parser_(make_parser(config))
{
    init();
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
                auto fields = parser_->fields();

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
            save_record(std::move(parser_->fields()));
            return true;
        }

        if (result == ParseStatus::fail) {
            break;
        }
    }

    return false; // on ParseStatus::fail
}

}