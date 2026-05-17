#include <iostream>
#include <memory>
#include <csvengine.hpp>

int main() {

    // --- SHOW LINE NUM WITH ERROR ---

    const std::string error_at_line_0 = "\"col 1\",\"col 2\",\"col 3\"\n"
        "\"This is OK\",This\"gives an error\",OK\n";

    csv::Reader reader1{
        std::make_unique<std::istringstream>(error_at_line_0),
        csv::Config{
            .has_quoting=true,
            .parse_mode=csv::Config::ParseMode::strict
        }
    };

    try {
        while(reader1.next());
    }
    catch (const csv::ParseError& err) {
        std::cout << err.what() << std::endl;
    }


    const std::string error_at_line_2 = "\"col 1\",\"col 2\",\"col 3\"\n"
        "\"This is OK\",This deosnt give an error,OK\n"
        "OK,OK,OK\n"
        "OK,OK,NOTOK\"\n"
        "OK,OK,OK\r\n";

    csv::Reader reader2 {
        std::make_unique<std::istringstream>(error_at_line_2),
        csv::Config{
            .has_quoting=true,
            .parse_mode=csv::Config::ParseMode::strict
        }
    };

    try {
        while(reader2.next());
    }
    catch (const csv::ParseError& err) {
        std::cout << err.what() << std::endl;
    }


    const std::string error_at_line_3_for_crlf = "\"col 1\",\"col 2\",\"col 3\"\r\n"
        "\"This is OK\",This deosnt give an error,OK\r\n"
        "OK,OK,OK\r\n"
        "OK,OK,OK\r\n"
        "OK,OK,NOTOK\n"
        "OK,OK,OK\r\n";

    csv::Reader reader3{
        std::make_unique<std::istringstream>(error_at_line_3_for_crlf),
        csv::Config{
            .has_quoting=true,
            .parse_mode=csv::Config::ParseMode::strict,
            .line_ending=csv::Config::LineEnding::crlf
        }
    };

    try {
        while(reader3.next());
    }
    catch (const csv::ParseError& err) {
        std::cout << err.what() << std::endl;
    }


    const std::string error_at_line_4_for_crlf = "\"col 1\",\"col 2\",\"col 3\"\n"
        "\"This is OK\",This deosnt give an error,OK\n"
        "OK,OK,OK\n"
        "OK,OK,OK\n"
        "OK,OK,OK\n"
        "OK,\"NOTO\"K,OK\n";
    csv::Reader reader4{
        std::make_unique<std::istringstream>(error_at_line_4_for_crlf),
        csv::Config{
            .has_quoting=true,
            .parse_mode=csv::Config::ParseMode::strict
        }
    };

    try {
        while(reader4.next());
    }
    catch (const csv::ParseError& err) {
        std::cout << err.what() << std::endl;
    }
    return 0;
}
