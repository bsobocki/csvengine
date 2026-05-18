#pragma once

#include <string>

const std::string simple_csv_data = R"(name,age,country
Ken Adams,18,USA
Cristiano Ronaldo,35,Portugal
Gunter Shmitt,45,Germany
Andrzej Kowalski,55,Poland
John Krasinski,40,USA)";

const std::string quoted_csv_data = R"("Product","Description","Price, but Netto"
"Widget A","Standard widget, 5"" diameter","$10.99"
"Widget ""Pro""","Professional grade, includes:
- Feature 1
- Feature 2","$49.99"
)";

// Quoted CSV without embedded newlines.
// Subset of RFC 4180 supported by ALL major C++ CSV libraries.
// Used for fair head-to-head benchmarks against fast-cpp-csv-parser etc.
const std::string quoted_simple_csv_data = R"("Product","Description","Price"
"Widget A","Standard widget, 5"" diameter","$10.99"
"Widget ""Pro""","Professional grade, premium quality","$49.99"
"Gadget X","Small, lightweight, portable","$5.50"
)";
