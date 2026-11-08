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
