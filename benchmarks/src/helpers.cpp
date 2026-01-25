#include <helpers.hpp>
#include <testdata.hpp>

namespace csv {

// Utility: repeat an input CSV N times.
// (Ensures bigger data sizes so benchmarks are meaningful.)
std::string repeat_csv(std::string_view one_file, int repeats) {
    std::string out;
    out.reserve(one_file.size() * repeats);
    
    if (one_file.empty()) return "";
    
    for (int i = 0; i < repeats; ++i) {
        out.append(one_file);
        // ensure last record ends with newline to avoid “EOF flush” differences
        if (one_file.back() != '\n') out.push_back('\n');
    }
    return out;
}

}