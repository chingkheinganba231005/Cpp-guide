General-purpose `new`/`malloc` must handle any size, any lifetime and any thread, so they can't be the fastest choice for every pattern. High-performance systems use **custom allocators** tuned to their allocation patterns. Game engines use per-frame arenas, trading engines use order pools that never touch the heap after start-up, and ML frameworks use caching allocators for tensors. In this project you build the two most important ones.

## Pattern 1: the arena (bump) allocator

Many allocations share one lifetime: everything allocated while processing a frame, a request or a parse tree can be freed together. An **arena** grabs one big block and hands out pieces by bumping a pointer forward. Freeing individual objects is impossible, and unnecessary: you reset the whole arena at once.

:::eli5
A notepad. Each allocation takes the next blank lines, and nobody erases single lines. At the end of the day you tear off all the used pages at once. Allocation is just “move the bookmark down”.
:::

```cpp file=arena.hpp
#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <utility>

class Arena {
public:
    explicit Arena(std::size_t bytes) : buf_(std::make_unique<std::byte[]>(bytes)), cap_(bytes) {}

    void* allocate(std::size_t size, std::size_t align = alignof(std::max_align_t)) {
        std::size_t p = (used_ + align - 1) & ~(align - 1);       // round up to the alignment (a power of two)
        if (p + size > cap_) throw std::bad_alloc();
        used_ = p + size;
        return buf_.get() + p;
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {                                     // construct an object in arena memory
        return new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);   // "placement new"
    }

    void reset() { used_ = 0; }          // free everything at once (objects must be trivially destructible, or destroyed first)
    std::size_t used() const { return used_; }

private:
    std::unique_ptr<std::byte[]> buf_;
    std::size_t cap_, used_ = 0;
};
```

**Placement new** (`new (address) T(args)`) constructs an object at an address you already own, without allocating. It is how every container and allocator builds objects in raw memory.

```cpp run with=arena.hpp
#include <chrono>
#include <iostream>
#include "arena.hpp"

struct Node { float x, y; Node* next; };

int main() {
    constexpr int N = 1'000'000, FRAMES = 20;
    auto t0 = std::chrono::steady_clock::now();
    for (int f = 0; f < FRAMES; ++f) {                       // general-purpose heap: new + delete each object
        Node* head = nullptr;
        for (int i = 0; i < N; ++i) head = new Node{1.f * i, 2.f, head};
        while (head) { Node* n = head->next; delete head; head = n; }
    }
    auto t1 = std::chrono::steady_clock::now();
    Arena arena(sizeof(Node) * N + 64);
    float check = 0;
    for (int f = 0; f < FRAMES; ++f) {                       // arena: bump-allocate, then free everything in O(1)
        Node* head = nullptr;
        for (int i = 0; i < N; ++i) head = arena.make<Node>(1.f * i, 2.f, head);
        check += head->x;
        arena.reset();
    }
    auto t2 = std::chrono::steady_clock::now();
    auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
    std::cout << "new/delete: " << ms(t0, t1) << " ms\narena:      " << ms(t1, t2) << " ms (" << static_cast<int>(ms(t0, t1) / ms(t1, t2)) << "x faster)\n";
    std::cout << "check " << check << '\n';
}
```

The arena wins because allocation is an addition and a comparison, freeing is one assignment, and the nodes sit contiguously in memory (cache-friendly, chapter 26).

## Pattern 2: the pool (free-list) allocator

When you allocate and free many objects **of one size** in any order (orders in a book, particles, network messages), use a **pool**: a pre-allocated array of slots, with the free slots linked into a list. Allocation pops the head of the list; freeing pushes the slot back. Both are O(1), with no system calls and no fragmentation.

```cpp file=pool.hpp
#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <vector>

template <typename T>
class Pool {
    union Slot { Slot* next; alignas(T) std::byte storage[sizeof(T)]; };   // a free slot stores the "next free" link
public:
    explicit Pool(std::size_t capacity) : slots_(capacity) {
        for (std::size_t i = 0; i + 1 < capacity; ++i) slots_[i].next = &slots_[i + 1];
        if (capacity) slots_[capacity - 1].next = nullptr;
        free_ = capacity ? &slots_[0] : nullptr;
    }
    template <typename... Args>
    T* create(Args&&... args) {
        if (!free_) throw std::bad_alloc();
        Slot* s = free_;
        free_ = s->next;                                  // pop
        ++live_;
        return new (s->storage) T(std::forward<Args>(args)...);
    }
    void destroy(T* p) {
        p->~T();                                          // run the destructor explicitly
        Slot* s = reinterpret_cast<Slot*>(p);
        s->next = free_;                                  // push
        free_ = s;
        --live_;
    }
    std::size_t live() const { return live_; }
private:
    std::vector<Slot> slots_;                             // allocated once, up front
    Slot* free_ = nullptr;
    std::size_t live_ = 0;
};
```

```cpp run with=pool.hpp
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include "pool.hpp"

struct Order { long id; long price; int qty; char side; };

int main() {
    constexpr int OPS = 5'000'000, LIVE = 10'000;
    std::mt19937 rng(42);
    auto bench = [&](auto make, auto kill) {
        std::vector<Order*> live; live.reserve(LIVE);
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < OPS; ++i) {
            if (live.size() < LIVE && (live.empty() || rng() % 2)) live.push_back(make(i));
            else { std::size_t k = rng() % live.size(); kill(live[k]); live[k] = live.back(); live.pop_back(); }   // free a random order
        }
        for (auto* o : live) kill(o);
        return std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / OPS;
    };
    double heap = bench([](int i) { return new Order{i, 100, 1, 'B'}; }, [](Order* o) { delete o; });
    Pool<Order> pool(LIVE);
    double pooled = bench([&](int i) { return pool.create(Order{i, 100, 1, 'B'}); }, [&](Order* o) { pool.destroy(o); });
    std::cout << "new/delete: " << heap << " ns per operation\npool:       " << pooled << " ns per operation\n";
    std::cout << "objects still alive in the pool: " << pool.live() << '\n';
}
```

Freeing happens in random order here, which is the realistic case for an order book, where any order can be cancelled at any time. Beyond the average speed, the pool's real advantage for low-latency work is **predictability**: no call ever enters the general allocator, so there are no occasional slow paths and no system calls.

## The standard way: polymorphic memory resources (std::pmr)

C++17's `<memory_resource>` lets standard containers use your allocation strategy without changing their type's interface:

```cpp run
#include <array>
#include <cstddef>
#include <iostream>
#include <memory_resource>
#include <string>
#include <vector>

int main() {
    std::array<std::byte, 64 * 1024> buffer;                          // 64 KB on the stack
    std::pmr::monotonic_buffer_resource arena(buffer.data(), buffer.size());   // an arena, provided by the library
    std::pmr::vector<std::pmr::string> names(&arena);                 // the vector AND its strings allocate from the arena
    for (int i = 0; i < 100; ++i) names.emplace_back("player_" + std::to_string(i) + "_with_a_long_name");
    std::cout << names.size() << " strings, last: " << names.back() << '\n';
    std::cout << "no heap allocation happened while the buffer had room\n";
}
```

`monotonic_buffer_resource` is an arena, `unsynchronized_pool_resource` is a set of pools, and you can write your own by deriving from `std::pmr::memory_resource`.

## Where allocators are used

| System | Allocator pattern |
|---|---|
| Game engine | per-frame arena (reset every frame), pools for entities and particles, stack allocators for level loading |
| Trading engine | pre-allocated pools for orders and messages: zero heap allocation after start-up |
| ML framework | a caching allocator for GPU tensors (PyTorch's CUDA caching allocator), arenas for graph nodes |
| Compiler / parser | an arena for the abstract syntax tree, freed in one go |
| Web server | a per-request arena |

:::exercise Make the arena safe for non-trivial types
`Arena::reset` doesn't run destructors, so objects such as `std::string` would leak their own heap memory. Add a static check that `make<T>` only accepts trivially destructible types, using `static_assert(std::is_trivially_destructible_v<T>)`. Then think about how you could support other types: record a destructor callback for each object and call them in reverse order in `reset()`.
---solution---
```cpp norun
template <typename T, typename... Args>
T* make(Args&&... args) {
    static_assert(std::is_trivially_destructible_v<T>, "Arena::make only supports trivially destructible types");
    return new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
}
```
For general types, store `{void* object, void (*destroy)(void*)}` pairs (in the arena itself) and call them from last to first in `reset()`. That is how some engine frame allocators work.
:::

::quiz
? What makes arena allocation so fast?
+ Allocating just moves a pointer forward, and everything is freed at once
- It uses the GPU
- It never runs out of memory
> There is no search for a free block and no per-object bookkeeping.
? When is a pool allocator the right choice?
+ Many objects of the same size, created and destroyed in any order
- Objects of wildly different sizes
- Objects that all live forever
> A free list of fixed-size slots gives O(1) create and destroy.
? What does placement new do?
+ Constructs an object at an address you already own, without allocating
- Allocates on the stack
- Frees memory
> new (ptr) T(args) runs T's constructor at ptr.
::
