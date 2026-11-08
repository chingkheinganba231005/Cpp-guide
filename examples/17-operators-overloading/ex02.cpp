#include <algorithm>
#include <compare>
#include <iostream>
#include <string>
#include <vector>

struct Version {
    int major = 0, minor = 0, patch = 0;
    auto operator<=>(const Version&) const = default;   // compares major, then minor, then patch
    bool operator==(const Version&) const = default;
};

int main() {
    std::vector<Version> vs{{1, 2, 3}, {1, 10, 0}, {0, 9, 9}, {1, 2, 0}};
    std::ranges::sort(vs);
    for (const auto& v : vs) std::cout << v.major << '.' << v.minor << '.' << v.patch << "  ";
    std::cout << '\n' << std::boolalpha << (Version{1, 2, 3} < Version{1, 10, 0}) << ' ' << (Version{1, 0, 0} == Version{1, 0, 0}) << '\n';
}
