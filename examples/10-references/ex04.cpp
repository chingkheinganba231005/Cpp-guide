#include <iostream>
#include <string>
#include <vector>

int main() {
    std::vector<std::string> names{"ada", "linus", "grace"};
    for (auto& n : names) n[0] = static_cast<char>(n[0] - 'a' + 'A');   // modify in place
    for (const auto& n : names) std::cout << n << ' ';                  // read without copying
    std::cout << '\n';
    for (auto n : names) n = "changed";      // modifies copies only: names is unchanged
    std::cout << names[0] << '\n';
}
