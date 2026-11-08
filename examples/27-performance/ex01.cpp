#include <chrono>
#include <iostream>
#include <vector>

int main() {
    const int N = 4096;
    std::vector<float> m(static_cast<size_t>(N) * N, 1.0f);    // 64 MB, stored row by row
    auto time = [&](bool by_rows) {
        auto t0 = std::chrono::steady_clock::now();
        double s = 0;
        for (int a = 0; a < N; ++a)
            for (int b = 0; b < N; ++b)
                s += by_rows ? m[static_cast<size_t>(a) * N + b]     // consecutive addresses
                             : m[static_cast<size_t>(b) * N + a];    // jumps 16 KB every step
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        return std::pair{s, ms};
    };
    auto [s1, t1] = time(true);
    auto [s2, t2] = time(false);
    std::cout << "row by row:       " << t1 << " ms\n";
    std::cout << "column by column: " << t2 << " ms  (" << static_cast<int>(t2 / t1 + 0.5) << "x slower, same arithmetic)\n";
    std::cout << (s1 == s2 ? "same sum\n" : "different\n");
}
