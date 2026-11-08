Matrix multiplication is the most important computation in scientific computing and machine learning: almost all the work in a neural network is matmuls. It is also the perfect case study in performance engineering, because the same O(n³) arithmetic can run **dozens of times faster** just by respecting the hardware. In this project you take it from naive to about 30× faster in four measured steps, then see how professional libraries go further.

C = A × B for n × n matrices: `C[i][j] = Σₖ A[i][k] · B[k][j]`. That is 2n³ floating-point operations (one multiply and one add per term). We measure speed in **GFLOP/s**: billions of floating-point operations per second.

## The benchmark harness

```cpp file=mm.hpp
#pragma once
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

using Mat = std::vector<float>;                                    // n*n, row-major

inline Mat random_matrix(int n, unsigned seed) {
    std::mt19937 rng(seed); std::uniform_real_distribution<float> d(-1, 1);
    Mat m(static_cast<size_t>(n) * n); for (auto& x : m) x = d(rng); return m;
}

template <typename F>
double bench(const char* name, F f, int n, const Mat& ref, Mat& c) {
    std::fill(c.begin(), c.end(), 0.0f);
    auto t0 = std::chrono::steady_clock::now();
    f(c);
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    double err = 0;
    for (size_t i = 0; i < c.size(); ++i) err = std::max(err, static_cast<double>(std::abs(c[i] - ref[i])));
    double gflops = 2.0 * n * n * n / s / 1e9;
    std::printf("%-28s %8.1f ms  %7.2f GFLOP/s   max error %.1e\n", name, s * 1e3, gflops, err);
    return gflops;
}
```

## The five versions

```cpp run with=mm.hpp flags="-O3 -march=native"
#include <algorithm>
#include <thread>
#include <vector>
#include "mm.hpp"

const int N = 768;

// 1. Naive i-j-k: B is walked down a column, a stride of N floats each step: a cache miss almost every time.
void naive(const Mat& A, const Mat& B, Mat& C) {
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            float s = 0;
            for (int k = 0; k < N; ++k) s += A[i * N + k] * B[k * N + j];
            C[i * N + j] = s;
        }
}

// 2. Loop interchange i-k-j: the inner loop walks rows of B and C contiguously, so the compiler vectorises it.
void ikj(const Mat& A, const Mat& B, Mat& C) {
    for (int i = 0; i < N; ++i)
        for (int k = 0; k < N; ++k) {
            float a = A[i * N + k];
            for (int j = 0; j < N; ++j) C[i * N + j] += a * B[k * N + j];
        }
}

// 3. Cache blocking (tiling): work on small blocks that stay in L1/L2 cache while they are reused.
void blocked(const Mat& A, const Mat& B, Mat& C, int i0 = 0, int i1 = N) {
    constexpr int BI = 64, BK = 128, BJ = 256;
    for (int ii = i0; ii < i1; ii += BI)
        for (int kk = 0; kk < N; kk += BK)
            for (int jj = 0; jj < N; jj += BJ)
                for (int i = ii; i < std::min(ii + BI, i1); ++i)
                    for (int k = kk; k < std::min(kk + BK, N); ++k) {
                        float a = A[i * N + k];
                        const float* b = &B[k * N + jj];
                        float* c = &C[i * N + jj];
                        int jn = std::min(BJ, N - jj);
                        for (int j = 0; j < jn; ++j) c[j] += a * b[j];
                    }
}

// 4. Multithreaded: each thread computes a band of rows of C. No sharing of output, so no locks.
void threaded(const Mat& A, const Mat& B, Mat& C) {
    unsigned T = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::jthread> ts;
    for (unsigned t = 0; t < T; ++t) {
        int i0 = N * t / T, i1 = N * (t + 1) / T;
        ts.emplace_back([&, i0, i1] { blocked(A, B, C, i0, i1); });
    }
}

int main() {
    Mat A = random_matrix(N, 1), B = random_matrix(N, 2), ref(static_cast<size_t>(N) * N), C(ref.size());
    naive(A, B, ref);                                             // reference result for checking
    std::printf("N = %d, %u hardware threads\n", N, std::thread::hardware_concurrency());
    double g1 = bench("1. naive (i-j-k)", [&](Mat& c) { naive(A, B, c); }, N, ref, C);
    double g2 = bench("2. loop interchange (i-k-j)", [&](Mat& c) { ikj(A, B, c); }, N, ref, C);
    double g3 = bench("3. + cache blocking", [&](Mat& c) { blocked(A, B, c); }, N, ref, C);
    double g4 = bench("4. + threads", [&](Mat& c) { threaded(A, B, c); }, N, ref, C);
    std::printf("speed-up from version 1 to version 4: %.0fx\n", g4 / g1);
    (void)g2; (void)g3;
}
```

(The small “max error” comes from adding floats in a different order: floating-point addition is not exactly associative.)

What each step changed:

1. **Naive**: correct but cache-hostile. The inner loop reads `B[k][j]` down a column, jumping N × 4 bytes each time (chapter 26's column-walk problem).
2. **Loop interchange**: the same maths with the loops reordered. Now the inner loop streams along rows, the hardware prefetcher keeps up, and the compiler emits SIMD instructions processing 8 floats at a time (`-march=native` enables AVX2).
3. **Blocking**: a block of B is reused for many rows of A while it is still in cache, reducing trips to slower memory.
4. **Threads**: split the rows of C between cores. Each thread writes only its own rows, so there is no synchronisation and no data race.

## How far can it go?

A CPU's peak is roughly *cores × clock × SIMD width × 2* (fused multiply-add). For 4 cores at 3 GHz with AVX2 that is about 4 × 3 × 8 × 2 × 2 ≈ 380 GFLOP/s (two FMA units per core). Our best version reached roughly 50 GFLOP/s on the machine that built this book: a big improvement, but only a modest fraction of peak. Libraries such as OpenBLAS, Intel MKL and BLIS reach 80–90% of peak with:

- **Register blocking (a micro-kernel)**: compute a small block of C (say 6×16) entirely in vector registers, loading each value of A and B once per block.
- **Packing**: copy blocks of A and B into contiguous, aligned buffers in exactly the order the micro-kernel reads them.
- **Explicit SIMD intrinsics** (`_mm256_fmadd_ps`) or assembly, tuned per CPU.
- **Multi-level blocking** sized for L1, L2 and L3.

GPUs apply the same ideas at massive scale. Chapter 32's tiled CUDA kernel is version 3 of this chapter, using shared memory as a hand-managed cache.

:::tip In real projects
Call a BLAS library (`cblas_sgemm`) or use Eigen rather than writing your own production matmul. But having done this once, you'll understand why layouts, strides and blocking dominate the design of every ML library and GPU kernel.
:::

:::exercise Write a micro-kernel
Write a kernel that computes a 4×8 block of C using eight `float` accumulators per row (32 in total), held in local variables: for each k, load four values of A and eight of B, and update all 32 accumulators. Call it from a loop over blocks. Compile with `-O3 -march=native` and look at the assembly in Compiler Explorer: you should see `vfmadd` instructions working on `ymm` registers.
---solution---
```cpp norun
void kernel_4x8(const float* A, const float* B, float* C, int n, int K) {
    float c[4][8] = {};
    for (int k = 0; k < K; ++k) {
        const float* b = B + k * n;
        for (int r = 0; r < 4; ++r) {
            float a = A[r * n + k];
            for (int j = 0; j < 8; ++j) c[r][j] += a * b[j];      // the compiler keeps c in registers
        }
    }
    for (int r = 0; r < 4; ++r) for (int j = 0; j < 8; ++j) C[r * n + j] += c[r][j];
}
```
:::

::quiz
? Why is the naive i-j-k loop slow?
+ Its inner loop walks a column of B, striding through memory, so most accesses miss the cache
- It does more arithmetic
- It uses doubles
> Same arithmetic; only the access pattern differs.
? What does cache blocking achieve?
+ It reuses small blocks of data many times while they are in fast cache
- It reduces the number of multiplications
- It removes the need for threads
> Fewer trips to slower memory for the same arithmetic.
::
