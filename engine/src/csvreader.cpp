#include <csvreader.hpp>

CSVReader::CSVReader(const std::string& filePath, const CSVConfig config): _config(config), _currentRecord(0) {
    _csvFile.open(filePath, std::ifstream::in);

    if (!_csvFile.good())
        throw std::runtime_error("CSVReader: cannot open " + filePath);
}

std::optional<CSVRecord> CSVReader::nextRecord() {

}

CSVReader::CSVIterator::CSVIterator(CSVReader* reader): _reader(reader) {
    if (_reader && _reader->lineNumber() == 0) {
        operator++();
    }
}

CSVReader::CSVIterator& CSVReader::CSVIterator::operator++() {
    if (_reader && !_reader->nextRecord())
        _reader = nullptr;

    return *this;
}

const CSVRecord& CSVReader::CSVIterator::operator*() const {
    return _reader->currentRecord();
}

bool CSVReader::CSVIterator::operator!=(const CSVIterator& other) const {
    return this->_reader != other._reader;
}

CSVReader::CSVIterator CSVReader::begin() {
    return CSVReader::CSVIterator(this);
}

CSVReader::CSVIterator CSVReader::end() {
    return CSVReader::CSVIterator(nullptr);
}

bool CSVReader::good() const {
    return _csvFile.good();
}

bool CSVReader::hasHeader() const {
    return _config.has_header;
}

std::size_t CSVReader::lineNumber () const {
    return _currentRecordIndex + 1;
}

CSVReader::operator bool() const {
    return good();
}

CSVConfig CSVReader::getConfig() const {
    return _config;
}

const CSVRecord& CSVReader::currentRecord() const {
    return _currentRecord;
}
    
const std::vector<std::string>& CSVReader::headers() const {
    return _headers;
}