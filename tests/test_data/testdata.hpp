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