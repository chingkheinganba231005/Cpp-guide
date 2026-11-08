#include <chrono>
#include <iostream>
#include <memory>

// Optimisers are clever: if a result is never used, they delete the work, even new/delete pairs.
// This empty assembly statement tells GCC/Clang "assume this pointer is used", so the work stays.
// (Benchmark libraries call it DoNotOptimize.)
inline void keep(const void* p) { asm volatile("" : : "g"(p) : "memory"); }

int main() {
    constexpr int N = 2'000'000;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) { int x = i; keep(&x); }                       // stack
    auto t1 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) { auto p = std::make_unique<int>(i); keep(p.get()); }   // heap, every time
    auto t2 = std::chrono::steady_clock::now();
    auto ns = [&](auto a, auto b) { return std::chrono::duration<double, std::nano>(b - a).count() / N; };
    std::cout << "stack: " << ns(t0, t1) << " ns per iteration\n";
    std::cout << "heap:  " << ns(t1, t2) << " ns per iteration (allocate + free)\n";
}
