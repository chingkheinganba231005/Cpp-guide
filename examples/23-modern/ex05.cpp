#include <array>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

double mean(std::span<const double> xs) {
    return xs.empty() ? 0.0 : std::accumulate(xs.begin(), xs.end(), 0.0) / xs.size();
}

int main() {
    std::vector<double> v{1, 2, 3, 4, 5, 6};
    std::array<double, 3> a{10, 20, 30};
    double c[] = {7, 8};
    std::cout << mean(v) << ' ' << mean(a) << ' ' << mean(c) << '\n';
    std::cout << mean(std::span(v).subspan(2, 3)) << " (elements 2..4 only, no copy)\n";
}
