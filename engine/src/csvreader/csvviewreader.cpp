#include <csvreader/csvreader.hpp>
#include <csvparser/csvparser.hpp>
#include <csverrors.hpp>
#include <csvbuffer/csvstreambuffer.hpp>
#include <csvbuffer/csvmappedbuffer.hpp>
#include <optional>
#include <format>

using namespace std;

namespace csv {

ViewReader::ViewReader(const std::string& filepath, const Config& config)
    : ReaderBase<RecordView>(filepath, config)
    , parser_(std::make_unique<ViewSimpleParser>(config))
{
    init();
}

ViewReader::ViewReader(std::unique_ptr<std::istream> stream, const Config& config)
    : ReaderBase<RecordView>(std::move(stream), config)
    , parser_(std::make_unique<ViewSimpleParser>(config))
{
    init();
}

ViewReader::ViewReader(std::unique_ptr<IBuffer> buffer, const Config& config)
    : ReaderBase<RecordView>(std::move(buffer), config)
    , parser_(std::make_unique<ViewSimpleParser>(config))
{
    init();
}

bool ViewReader::next() {
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
                throw RecordTooLargeError();
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

}