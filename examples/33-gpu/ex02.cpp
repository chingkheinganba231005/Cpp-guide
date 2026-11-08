#include <iostream>
#include <vector>

struct Dim { int x; };

// "Launch" a kernel on the CPU: loop over every block and every thread in the block
template <typename Kernel>
void launch(Dim grid, Dim block, Kernel k) {
    for (int b = 0; b < grid.x; ++b)
        for (int t = 0; t < block.x; ++t) k(Dim{b}, Dim{t}, block);
}

int main() {
    const int n = 10;
    std::vector<float> a(n, 1.0f), b(n), c(n);
    for (int i = 0; i < n; ++i) b[i] = static_cast<float>(i);
    int threads = 4, blocks = (n + threads - 1) / threads;               // 3 blocks of 4 threads = 12 threads for 10 elements
    launch({blocks}, {threads}, [&](Dim blockIdx, Dim threadIdx, Dim blockDim) {
        int i = blockIdx.x * blockDim.x + threadIdx.x;
        if (i < n) c[i] = a[i] + b[i];
        if (blockIdx.x == 2) std::cout << "block 2, thread " << threadIdx.x << " -> element " << i << (i < n ? "" : " (out of range: does nothing)") << '\n';
    });
    for (float x : c) std::cout << x << ' ';
    std::cout << '\n';
}
