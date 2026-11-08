#include <iostream>
#include <numeric>
#include <vector>
#include <chrono>

int main() {
    std::vector<double> v(1'000'000, 0.5);                 // a million numbers, each 0.5
    auto t0 = std::chrono::steady_clock::now();
    double total = std::accumulate(v.begin(), v.end(), 0.0);  // a high-level algorithm
    auto t1 = std::chrono::steady_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "total = " << total << " in about " << (us < 5000 ? "a millisecond or less" : "a few milliseconds") << '\n';
}
