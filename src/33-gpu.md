A GPU is a processor built for **throughput**: thousands of simple cores running the same program on different data. A modern GPU does tens to hundreds of trillions of floating-point operations per second, which is why it trains neural networks, renders games and runs simulations. This chapter teaches **CUDA**, NVIDIA's C++-based GPU language and the dominant one in ML, and how to write your own kernels.

:::warn About the code in this chapter
The machine that builds this book has no GPU, so the CUDA code below is **shown, not run** (marked “not run here”). It is standard CUDA C++ that compiles with NVIDIA's `nvcc`. Run it in Google Colab (free GPU: write it to a file with `%%writefile k.cu`, then `!nvcc k.cu -o k && ./k`) or on any machine with an NVIDIA GPU. The CPU simulation of the thread model *is* compiled and run.
:::

## CPU vs GPU

| | CPU | GPU |
|---|---|---|
| Cores | 4–64 complex cores | thousands of simple cores, grouped into SMs (streaming multiprocessors) |
| Good at | low-latency, branchy, serial work | the same operation on millions of elements |
| Hides memory latency with | big caches, prediction | huge numbers of threads: while some wait for memory, others compute |
| Memory | system RAM (~50–100 GB/s) | HBM/GDDR (1–3 TB/s, and more) |

## The CUDA programming model

You write a **kernel**: a function that runs on the GPU, executed by many threads at once. Threads are grouped into **blocks**, and blocks form a **grid**. Each thread knows its position and uses it to pick its piece of data:

```cuda norun
// vector_add.cu : C = A + B, one thread per element
#include <cstdio>
#include <vector>

__global__ void add(const float* a, const float* b, float* c, int n) {     // __global__: a GPU kernel
    int i = blockIdx.x * blockDim.x + threadIdx.x;                          // this thread's global index
    if (i < n) c[i] = a[i] + b[i];                                          // guard: the grid may overshoot n
}

int main() {
    int n = 1 << 24;
    std::vector<float> ha(n, 1.0f), hb(n, 2.0f), hc(n);
    float *da, *db, *dc;                                                    // device (GPU) pointers
    cudaMalloc(&da, n * sizeof(float)); cudaMalloc(&db, n * sizeof(float)); cudaMalloc(&dc, n * sizeof(float));
    cudaMemcpy(da, ha.data(), n * sizeof(float), cudaMemcpyHostToDevice);   // copy inputs to the GPU
    cudaMemcpy(db, hb.data(), n * sizeof(float), cudaMemcpyHostToDevice);
    int threads = 256, blocks = (n + threads - 1) / threads;                // enough blocks to cover n
    add<<<blocks, threads>>>(da, db, dc, n);                                // launch: blocks x threads
    cudaMemcpy(hc.data(), dc, n * sizeof(float), cudaMemcpyDeviceToHost);   // copy the result back (waits for the kernel)
    std::printf("c[0] = %f\n", hc[0]);
    cudaFree(da); cudaFree(db); cudaFree(dc);
}
```

The key ideas:

- `__global__` marks a kernel; `<<<blocks, threads>>>` launches it.
- `blockIdx`, `blockDim` and `threadIdx` give each thread its coordinates (in up to 3 dimensions).
- GPU memory is separate: `cudaMalloc` allocates it and `cudaMemcpy` copies to and from it. **Transfers are slow** compared with GPU compute, so keep data on the GPU across many kernels. That is why PyTorch tensors have a `.device`.

The same indexing, simulated on the CPU so you can see who does what:

```cpp run
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
```

On a real GPU these 12 “iterations” run **simultaneously**. Threads execute in groups of 32 called **warps**, which run in lockstep: every thread in a warp executes the same instruction. That leads to two performance rules:

- **Coalesced memory access**: when the 32 threads of a warp read 32 *consecutive* floats, the hardware combines them into one wide memory transaction. Scattered addresses cost up to 32 separate transactions. This is chapter 26's “contiguous is fast”, at warp scale.
- **Avoid divergence**: if threads in one warp take different branches of an `if`, the warp runs both branches one after the other.

## The GPU memory hierarchy

| Memory | Scope | Speed | Use |
|---|---|---|---|
| Registers | one thread | fastest | local variables |
| **Shared memory** | one block | on-chip: far lower latency than global memory | a programmer-managed cache for data reused within a block |
| L1/L2 cache | SM / whole GPU | fast | automatic |
| Global memory (HBM/GDDR) | all threads, plus the host via copies | slowest on-chip path | inputs and outputs |

## Kernel 2: tiled matrix multiplication with shared memory

The naive GPU matmul reads every element of A and B from global memory N times. The **tiled** version loads a TILE×TILE block of A and B into shared memory once, synchronises, and lets every thread in the block reuse it. This is chapter 31's cache blocking, done by hand:

```cuda norun
#define TILE 32

__global__ void matmul_tiled(const float* A, const float* B, float* C, int N) {
    __shared__ float As[TILE][TILE];                  // shared by the whole block
    __shared__ float Bs[TILE][TILE];
    int row = blockIdx.y * TILE + threadIdx.y;
    int col = blockIdx.x * TILE + threadIdx.x;
    float acc = 0.0f;
    for (int t = 0; t < (N + TILE - 1) / TILE; ++t) {
        int a_col = t * TILE + threadIdx.x, b_row = t * TILE + threadIdx.y;
        As[threadIdx.y][threadIdx.x] = (row < N && a_col < N) ? A[row * N + a_col] : 0.0f;   // each thread loads ONE element (coalesced)
        Bs[threadIdx.y][threadIdx.x] = (b_row < N && col < N) ? B[b_row * N + col] : 0.0f;
        __syncthreads();                              // wait until the whole tile is loaded
        for (int k = 0; k < TILE; ++k) acc += As[threadIdx.y][k] * Bs[k][threadIdx.x];       // reuse from fast shared memory
        __syncthreads();                              // wait before overwriting the tile
    }
    if (row < N && col < N) C[row * N + col] = acc;
}
// launch: dim3 block(TILE, TILE); dim3 grid((N + TILE - 1) / TILE, (N + TILE - 1) / TILE);
//         matmul_tiled<<<grid, block>>>(dA, dB, dC, N);
```

`__syncthreads()` is a **barrier**: no thread in the block continues until all have reached it. Without the first one, threads would read tiles that aren't fully loaded yet; without the second, a fast thread could overwrite a tile that others are still using. The tiled kernel is typically several times faster than the naive one. cuBLAS adds register tiling, tensor cores and much more on top.

## Kernel 3: parallel reduction (summing an array)

Summing is harder to parallelise because every element contributes to one result. The classic GPU pattern is a **tree reduction**: each block sums its part in shared memory, halving the number of active threads each step, then the block totals are combined:

```cuda norun
__global__ void block_sum(const float* in, float* block_totals, int n) {
    extern __shared__ float s[];                      // size given at launch
    int tid = threadIdx.x, i = blockIdx.x * blockDim.x * 2 + tid;
    float v = 0;
    if (i < n) v += in[i];
    if (i + blockDim.x < n) v += in[i + blockDim.x];  // each thread first adds two elements
    s[tid] = v;
    __syncthreads();
    for (int stride = blockDim.x / 2; stride > 32; stride >>= 1) {     // tree: 256 -> 128 -> 64 partial sums
        if (tid < stride) s[tid] += s[tid + stride];
        __syncthreads();
    }
    if (tid < 32) {                                   // the last warp finishes with warp shuffles: no shared memory, no barrier
        float x = s[tid] + s[tid + 32];
        for (int off = 16; off > 0; off >>= 1) x += __shfl_down_sync(0xffffffff, x, off);
        if (tid == 0) block_totals[blockIdx.x] = x;
    }
}
// launch with blockDim.x = 256: block_sum<<<blocks, 256, 256 * sizeof(float)>>>(d_in, d_partial, n);
// then sum the per-block totals (a second launch, or on the CPU if there are few blocks)
```

Reductions, scans (prefix sums) and matmuls are the three patterns behind most GPU code: softmax, layer norm and attention are built from them.

## Writing your own ML kernels

| Path | What it is |
|---|---|
| **CUDA C++** | full control: what cuBLAS, cuDNN, FlashAttention and llama.cpp's GPU code are written in |
| **PyTorch C++/CUDA extensions** | write `kernel.cu` plus a small binding (chapter 30) and call it on tensors from Python |
| **Triton** | write GPU kernels in Python-like syntax; it generates the tiling, used heavily in PyTorch 2 |
| **CUTLASS / CuTe** | NVIDIA's C++ template library for building high-performance matmul and attention kernels |
| **HIP** (AMD), **SYCL** (Intel and others), **Metal** (Apple), **Vulkan compute / WebGPU** | the same concepts on other hardware |

**Kernel fusion** is the most common reason to write your own kernel. `y = gelu(x * w + b)` run as three separate kernels reads and writes the whole tensor from global memory three times; one fused kernel does it once, which can be 2–3× faster for memory-bound operations. FlashAttention is the famous example: it fuses attention into one tiled kernel that never writes the huge attention matrix to memory.

## How to measure a kernel

- Time it with `cudaEvent_t` (`cudaEventRecord` before and after, then `cudaEventElapsedTime`), because kernel launches are asynchronous.
- Compare against the hardware limits: achieved FLOP/s vs peak FLOP/s, and achieved GB/s vs peak memory bandwidth. Most simple kernels are **memory-bound**.
- Profile with **Nsight Compute** (per-kernel detail: occupancy, memory throughput, warp stalls) and **Nsight Systems** (the whole timeline: CPU, GPU and transfers).

::quiz
? In a CUDA kernel, how does each thread find its element?
+ From blockIdx, blockDim and threadIdx: i = blockIdx.x * blockDim.x + threadIdx.x
- It receives an index argument
- Threads share one index
> The grid of blocks of threads maps onto the data.
? What is shared memory used for in the tiled matmul?
+ A fast, block-wide cache for tiles of A and B that every thread in the block reuses
- Communication between the CPU and the GPU
- Storing the final result
> It cuts global-memory traffic by a factor of the tile size.
? Why fuse several element-wise operations into one kernel?
+ To read and write the data from global memory once instead of once per operation
- Because the GPU can only run one kernel
- To use more registers
> Memory-bound operations are limited by data movement, not arithmetic.
::
