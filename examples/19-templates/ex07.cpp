#include <array>
#include <iostream>
#include <vector>

template <typename Container>
double mean(const Container& c) {
    double sum = 0;
    std::size_t n = 0;
    for (const auto& x : c) { sum += x; ++n; }
    return n ? sum / n : 0.0;
}

int main() {
    std::vector<int> a{1, 2, 3, 4};
    std::vector<double> b{0.5, 1.5};
    std::array<float, 3> c{1.f, 1.f, 4.f};
    std::cout << mean(a) << ' ' << mean(b) << ' ' << mean(c) << '\n';
}
