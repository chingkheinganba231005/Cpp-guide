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
