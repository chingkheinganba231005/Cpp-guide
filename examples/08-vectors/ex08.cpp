#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    std::vector<double> xs;
    for (double x; std::cin >> x;) xs.push_back(x);
    if (xs.empty()) return 0;
    auto sorted = xs;
    std::ranges::sort(sorted);
    double mean = std::accumulate(xs.begin(), xs.end(), 0.0) / xs.size();
    std::size_t n = sorted.size();
    double median = n % 2 ? sorted[n / 2] : (sorted[n / 2 - 1] + sorted[n / 2]) / 2;
    std::cout << "n=" << n << " mean=" << mean << " min=" << sorted.front() << " max=" << sorted.back() << " median=" << median << '\n';
}
