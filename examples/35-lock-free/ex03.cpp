#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

struct Packed { std::atomic<long> a{0}; std::atomic<long> b{0}; };                    // a and b on the same cache line
struct Padded { alignas(64) std::atomic<long> a{0}; alignas(64) std::atomic<long> b{0}; };   // separate lines

template <typename S>
double run() {
    S s;
    auto t0 = std::chrono::steady_clock::now();
    {
        std::jthread x([&] { for (int i = 0; i < 20'000'000; ++i) s.a.fetch_add(1, std::memory_order_relaxed); });
        std::jthread y([&] { for (int i = 0; i < 20'000'000; ++i) s.b.fetch_add(1, std::memory_order_relaxed); });
    }
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

int main() {
    double packed = run<Packed>(), padded = run<Padded>();
    std::printf("same cache line: %.0f ms\nseparate lines:  %.0f ms  (%.1fx faster)\n", packed, padded, packed / padded);
}
