Writing fast C++ is mostly about understanding the hardware: how the CPU fetches data, how it predicts branches, and how it does several calculations at once. This chapter explains the few ideas that matter most, and how to **measure** instead of guess. These are the foundations of the engine, ML, matmul and trading projects.

## The memory hierarchy: the biggest idea

A modern CPU can do several arithmetic operations every nanosecond, but fetching data from main memory (RAM) takes about **100 nanoseconds**. To hide that gap, CPUs have small, fast **caches** close to the cores:

| Level | Typical size | Latency | Analogy |
|---|---|---|---|
| Registers | a few hundred bytes | 0 cycles | in your hand |
| L1 cache | 32–64 KB per core | ~1 ns (4 cycles) | on your desk |
| L2 cache | 0.5–2 MB per core | ~4 ns | in the drawer |
| L3 cache | 8–64 MB shared | ~12–40 ns | in the next room |
| RAM | 8–128 GB | ~80–120 ns | across town |

Memory moves into the cache in **cache lines** of 64 bytes. Reading one `float` fetches its 15 neighbours too. The CPU also **prefetches**: if you walk memory in order, it fetches the next lines before you ask. So **contiguous, sequential access is fast, and scattered access is slow**.

<div data-widget="cache" data-title="Why traversal order matters"></div>

Measured on real hardware, summing the same 2D array two ways:

```cpp run
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
```

The same number of additions, the same result, and a large gap in speed, purely from the access pattern.

## Data-oriented design: AoS vs SoA

Object-oriented habits group data by *object* (an **array of structs**, AoS). But a loop usually needs only *some* fields of *every* object. Grouping data by *field* (a **struct of arrays**, SoA) means each cache line carries only useful data:

```cpp run
#include <chrono>
#include <iostream>
#include <vector>

struct ParticleAoS { float x, y, z, vx, vy, vz, mass, charge, r, g, b, a, age, life, id, flags; };   // 64 bytes

struct ParticlesSoA {                                       // one array per field
    std::vector<float> x, y, z, vx, vy, vz;
    explicit ParticlesSoA(size_t n) : x(n), y(n), z(n), vx(n, 1), vy(n, 1), vz(n, 1) {}
};

int main() {
    const size_t n = 4'000'000;
    const float dt = 0.016f;
    std::vector<ParticleAoS> aos(n);
    for (auto& p : aos) { p.vx = p.vy = p.vz = 1; }
    ParticlesSoA soa(n);

    auto t0 = std::chrono::steady_clock::now();
    for (int rep = 0; rep < 10; ++rep)
        for (auto& p : aos) { p.x += p.vx * dt; p.y += p.vy * dt; p.z += p.vz * dt; }     // uses 24 of every 64 bytes loaded
    auto t1 = std::chrono::steady_clock::now();
    for (int rep = 0; rep < 10; ++rep)
        for (size_t i = 0; i < n; ++i) { soa.x[i] += soa.vx[i] * dt; soa.y[i] += soa.vy[i] * dt; soa.z[i] += soa.vz[i] * dt; }
    auto t2 = std::chrono::steady_clock::now();

    auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
    std::cout << "AoS: " << ms(t0, t1) << " ms, SoA: " << ms(t1, t2) << " ms\n";
    std::cout << "check: " << aos[7].x << ' ' << soa.x[7] << '\n';
}
```

This is the idea behind **entity-component systems** in game engines (chapter 28), columnar databases, and tensor layouts in ML libraries.

## Branch prediction

The CPU guesses which way an `if` will go and starts working ahead. A wrong guess costs about 15–20 cycles. Predictable branches are almost free; random ones are expensive. This example turns off two optimisations (`-fno-if-conversion`, `-fno-tree-vectorize`) so that the `if` really stays a branch in the machine code:

```cpp run flags="-fno-if-conversion -fno-if-conversion2 -fno-tree-vectorize"
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
```

Same data, same additions: the sorted version is several times faster, only because the CPU's guesses are right. Now the twist. Compiled with plain `-O2`, both versions took about 78 ms on the machine that built this book: GCC replaced the `if` with branch-free code (a conditional move and vector instructions), so there was nothing left to mispredict. The optimiser often fixes simple cases for you. In complicated hot code it can't, so prefer predictable data and branch-free formulations such as `s += (x >= 128) * x`, and check the assembly in Compiler Explorer when it matters.

## SIMD: one instruction, many numbers

Modern CPUs have **vector registers** that hold 4, 8 or 16 floats and process them with one instruction (SSE, AVX and AVX-512 on x86; NEON on ARM). The compiler **auto-vectorises** simple loops at `-O2`/`-O3`, especially with `-march=native` (use your CPU's full instruction set):

```cpp run flags="-O3 -march=native"
#include <chrono>
#include <iostream>
#include <vector>

void saxpy(float a, const float* x, float* __restrict y, std::size_t n) {   // y = a*x + y: a classic BLAS kernel
    for (std::size_t i = 0; i < n; ++i) y[i] = a * x[i] + y[i];             // auto-vectorised: 8 floats per instruction with AVX
}

int main() {
    const std::size_t n = 1 << 24;
    std::vector<float> x(n, 1.0f), y(n, 2.0f);
    auto t0 = std::chrono::steady_clock::now();
    for (int r = 0; r < 20; ++r) saxpy(0.5f, x.data(), y.data(), n);
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    double gb = 20.0 * n * sizeof(float) * 3 / 1e9;                         // bytes read and written
    std::cout << "y[0] = " << y[0] << ", " << gb / s << " GB/s of memory traffic\n";
}
```

The result is limited by **memory bandwidth**, not arithmetic: the CPU can multiply faster than RAM can deliver numbers. Recognising whether code is *memory-bound* or *compute-bound* is a key skill; chapter 31 builds on it. `__restrict` promises the compiler that `x` and `y` don't overlap, which lets it vectorise freely.

## Measuring properly

1. **Build optimised** (`-O2`/`-O3`, `-DNDEBUG`). Debug builds can be 10–50× slower and mislead you.
2. **Benchmark with a harness** such as [Google Benchmark](https://github.com/google/benchmark): it repeats, warms up, and stops the optimiser removing your code (chapter 11's `keep` trick is `benchmark::DoNotOptimize`).
3. **Profile before optimising.** `perf record ./prog` then `perf report` on Linux; Instruments on macOS; Visual Studio's profiler or VTune on Windows. They show where time is actually spent, which is rarely where you guessed.
4. **Report distributions, not just averages**, especially for latency (chapter 33): the median and the 99th and 99.9th percentiles.
5. **Change one thing at a time** and keep the measurement.

## Optimisation priorities

1. **Algorithm**: O(n log n) beats a fast O(n²).
2. **Memory layout and access patterns**: contiguous data, SoA for hot loops, fewer pointers.
3. **Avoid needless work**: allocations in hot loops, copies (use `const&` and moves), repeated lookups.
4. **Help the compiler**: `-O3 -march=native`, simple loops it can vectorise, `const` and `__restrict`.
5. **Parallelism**: threads (chapter 25), then SIMD intrinsics, then GPUs (chapter 32).

::quiz
? Why is walking a row-major 2D array column by column slow?
+ Each access jumps to a different cache line, so most accesses miss the cache
- Columns need more arithmetic
- The compiler forbids it
> The data is the same; only the access pattern changes.
? A loop updates x, y and z of 4 million 64-byte particle structs. Why can SoA be faster?
+ Each loaded cache line contains only the fields the loop needs
- SoA uses fewer instructions
- SoA avoids floating-point maths
> AoS drags unused fields through the cache.
? Which build should you benchmark?
+ An optimised build such as -O2 or -O3 with -DNDEBUG
- A debug build with -O0
- Any build
> Debug builds measure the wrong thing.
::
