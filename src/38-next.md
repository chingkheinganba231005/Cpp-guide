You have travelled from `int main()` to memory and ownership, modern C++, concurrency, performance engineering, and real systems: allocators, an ECS, a rasteriser, autograd, a fast matmul, GPU kernels, a matching engine, lock-free queues and sockets. This last chapter gives you a map for going deeper in each direction.

## How to keep improving

1. **Build things**, then rebuild them faster. Every project in Part V and Part VI has an extension exercise; finish those first.
2. **Read great code.** Pick a well-written codebase in your area and read it slowly: EnTT (ECS), raylib (games), llama.cpp (ML inference), Abseil and folly (libraries), the LLVM sources (compilers).
3. **Measure everything.** Keep a benchmark next to every performance claim you make, as this course did.
4. **Watch talks.** CppCon, C++Now and Meeting C++ publish their talks for free. Start with Chandler Carruth on performance, Mike Acton's “Data-Oriented Design and C++”, and Herb Sutter on modern C++.

## Roadmaps by goal

### Games and engines
- **Learn**: SDL3 or GLFW and **OpenGL 4.x** first (simplest to start), then **Vulkan** (or DirectX 12 / Metal) for full control. Read LearnOpenGL.com and *Real-Time Rendering*.
- **Engine architecture**: *Game Engine Architecture* (Gregory); *Game Programming Patterns* (Nystrom, free online).
- **Physics**: *Game Physics Engine Development* (Millington); study Box2D and Jolt Physics.
- **Projects**: extend chapter 28 into a 2D platformer with raylib; port chapter 29's rasteriser to a GPU with OpenGL; add a job system and a Dear ImGui editor.

### Custom engines and graphics
- **Graphics programming**: *Physically Based Rendering* (Pharr, Jakob and Humphreys, free online), *Ray Tracing in One Weekend* (free).
- **Projects**: a path tracer with BVH acceleration and multithreading; a Vulkan renderer with deferred shading.

### Machine-learning systems and libraries
- **Learn**: the PyTorch internals (ATen, the dispatcher, autograd), and read llama.cpp and ggml for compact, practical ML code.
- **Numerical computing**: Eigen; BLAS and LAPACK; *What Every Programmer Should Know About Memory* (Drepper).
- **Projects**: grow chapter 30 into a tensor library with broadcasting, a tape-based autograd and a CPU matmul backend from chapter 31; train an MLP on MNIST; write a PyTorch C++ extension.

### GPU programming and kernels
- **Learn**: *Programming Massively Parallel Processors* (Hwu, Kirk and El Hajj); NVIDIA's CUDA C++ Programming Guide; then CUTLASS/CuTe and Triton.
- **Projects**: implement chapter 32's kernels and measure them against cuBLAS; write a fused softmax, then a FlashAttention-style tiled attention kernel; profile everything with Nsight Compute.

### Low-latency trading and finance
- **Learn**: market microstructure (*Trading and Exchanges*, Harris); exchange protocols (FIX, ITCH, SBE); Linux performance (`perf`, CPU isolation, huge pages); CppCon talks by Carl Cook (“When a Microsecond Is an Eternity”) and others.
- **Projects**: extend chapter 33 with order modification and IOC orders; feed it with a parser for real ITCH sample data; connect the book, a strategy and a logger with chapter 34's queues; measure end-to-end latency from packet to order.

### Systems programming
- **Learn**: *The Linux Programming Interface* (Kerrisk); *Computer Systems: A Programmer's Perspective* (Bryant and O'Hallaron); Asio for networking.
- **Projects**: an HTTP server with `epoll` (or io_uring); a key-value store with a write-ahead log and memory-mapped files; a small interpreter or compiler front end.

## Books for C++ itself

| Level | Book |
|---|---|
| Foundations | *A Tour of C++* (Stroustrup, 3rd ed.), a fast overview of modern C++ |
| Thorough | *C++ Primer* (Lippman, Lajoie and Moo); *Programming: Principles and Practice Using C++* (Stroustrup, 3rd ed.) |
| Modern idioms | *Effective Modern C++* (Meyers); *C++ Move Semantics* and *C++17 – The Complete Guide* (Josuttis) |
| Templates | *C++ Templates: The Complete Guide* (Vandevoorde, Josuttis and Gregor) |
| Concurrency | *C++ Concurrency in Action* (Williams, 2nd ed.) |
| Performance | *Optimized C++* (Guntheroth); Agner Fog's free optimisation manuals |
| Reference | [cppreference.com](https://en.cppreference.com): bookmark it and use it daily |

## Five capstone projects

1. **A 2D game with your own engine**: ECS, fixed timestep, sprite batching with OpenGL, sound, an editor built with Dear ImGui.
2. **A deep-learning library**: tensors with strides and broadcasting, autograd, a blocked and threaded CPU matmul, optional CUDA kernels, then train a small convolutional network on MNIST.
3. **A real-time ray tracer**: BVH, multithreading, SIMD, and a denoiser; compare CPU and GPU versions.
4. **A trading-system skeleton**: a binary market-data parser, order books for many symbols, a toy strategy, risk checks, a simulated exchange, and end-to-end latency percentiles.
5. **A database engine**: a B-tree on memory-mapped pages, a write-ahead log, a simple SQL parser, and concurrent readers.

Each of them uses everything in this course. Pick the one that excites you most, write a short design document first, and build it in small, tested, measured steps.

Good luck, and enjoy C++.
