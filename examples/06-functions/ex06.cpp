#include <iostream>
#include <string>

std::string repeat(const std::string& s, int times = 2, const std::string& sep = " ") {
    std::string out;
    for (int i = 0; i < times; ++i) out += (i ? sep : "") + s;
    return out;
}

int main() {
    std::cout << repeat("ha") << " | " << repeat("ha", 4) << " | " << repeat("ha", 3, "-") << '\n';
}
