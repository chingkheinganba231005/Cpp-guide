#include <chrono>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>

template <typename C>
double time_sum(const C& c) {
    auto t0 = std::chrono::steady_clock::now();
    long long s = std::accumulate(c.begin(), c.end(), 0LL);
    auto t1 = std::chrono::steady_clock::now();
    if (s == 42) std::cout << "";                    // use the result
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    const int n = 5'000'000;
    std::vector<int> v(n, 1);
    std::list<int> l(v.begin(), v.end());
    double tv = time_sum(v), tl = time_sum(l);
    std::cout << "vector: " << tv << " ms, list: " << tl << " ms (" << static_cast<int>(tl / tv) << "x slower)\n";
}
