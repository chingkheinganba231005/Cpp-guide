#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

int main() {
    std::vector<int> v(10'000'000);
    std::mt19937 rng(1);
    for (int& x : v) x = static_cast<int>(rng() % 256);
    auto run = [&](const char* label) {
        auto t0 = std::chrono::steady_clock::now();
        long long s = 0;
        for (int rep = 0; rep < 5; ++rep)
            for (int x : v) if (x >= 128) s += x;          // a branch the CPU must predict
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        std::cout << label << ms << " ms (sum " << s << ")\n";
    };
    run("random order: ");
    std::ranges::sort(v);
    run("sorted order: ");
}
