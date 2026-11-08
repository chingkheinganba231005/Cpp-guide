#include <iostream>
#include <limits>

int main() {
    std::cout << "largest int:  " << std::numeric_limits<int>::max() << '\n';
    unsigned int u = 0;
    u = u - 1;                       // unsigned arithmetic wraps around (well defined)
    std::cout << "0u - 1 = " << u << '\n';
    long long safe = 2'000'000'000LL * 2;   // use a wider type when results may be big
    std::cout << "2e9 * 2 as long long = " << safe << '\n';
}
