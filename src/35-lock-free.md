Low-latency systems are pipelines of threads, each pinned to its own core: one reads the network, one updates order books, one runs the strategy, one sends orders, one writes logs. They pass messages through **queues**. A mutex-protected queue can make a thread sleep and wait for the operating system, which costs microseconds. In this project you build the standard alternative: a **lock-free single-producer, single-consumer (SPSC) ring buffer**, the workhorse of trading systems, audio engines and game engines.

## The ring buffer

A ring buffer is a fixed array used in a circle. The producer writes at `tail` and moves it forward; the consumer reads at `head` and moves it forward. With exactly one producer and one consumer, each index is **written by only one thread**. That is the key that makes it possible without locks:

- Only the producer writes `tail`; the consumer only reads it.
- Only the consumer writes `head`; the producer only reads it.
- Making the indices atomic, with **release/acquire** ordering (chapter 25), guarantees that when the consumer sees the new `tail`, it also sees the data written before it.

```cpp file=spsc.hpp
#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <optional>

template <typename T, std::size_t N>           // N must be a power of two: wrap-around becomes a cheap bit-mask
class SpscQueue {
    static_assert((N & (N - 1)) == 0, "capacity must be a power of two");
public:
    bool push(const T& v) {                     // called ONLY by the producer thread
        std::size_t t = tail_.load(std::memory_order_relaxed);
        if (t - head_cache_ == N) {                                   // looks full: refresh our copy of head
            head_cache_ = head_.load(std::memory_order_acquire);
            if (t - head_cache_ == N) return false;                   // really full
        }
        buf_[t & (N - 1)] = v;                                         // 1. write the data
        tail_.store(t + 1, std::memory_order_release);                 // 2. publish it
        return true;
    }
    std::optional<T> pop() {                    // called ONLY by the consumer thread
        std::size_t h = head_.load(std::memory_order_relaxed);
        if (h == tail_cache_) {                                        // looks empty: refresh our copy of tail
            tail_cache_ = tail_.load(std::memory_order_acquire);
            if (h == tail_cache_) return std::nullopt;                 // really empty
        }
        T v = buf_[h & (N - 1)];                                       // the acquire above guarantees this data is visible
        head_.store(h + 1, std::memory_order_release);                 // hand the slot back to the producer
        return v;
    }
private:
    // Each index on its own cache line, so the two cores don't fight over one line (false sharing, below).
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::size_t tail_cache_ = 0;          // consumer's private copy of tail
    alignas(64) std::atomic<std::size_t> tail_{0};
    alignas(64) std::size_t head_cache_ = 0;          // producer's private copy of head
    alignas(64) std::array<T, N> buf_{};
};
```

Two professional details:

- **Cached indices**: reading the other thread's atomic index means pulling its cache line from the other core (tens of nanoseconds). Each side keeps a private copy and refreshes it only when the queue *looks* full or empty.
- **alignas(64)**: puts each index on its own 64-byte cache line (see false sharing below).

## Throughput and latency between two cores

```cpp run with=spsc.hpp flags="-O3 -march=native"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "spsc.hpp"

using Clock = std::chrono::steady_clock;
struct Msg { std::int64_t sent_ns; std::uint64_t seq; };

static std::int64_t now_ns() { return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count(); }

int main() {
    static SpscQueue<Msg, 1024> q;

    // 1. Throughput: the producer pushes as fast as it can
    constexpr std::uint64_t N = 5'000'000;
    auto t0 = Clock::now();
    {
        std::jthread consumer([] { for (std::uint64_t got = 0; got < N;) if (q.pop()) ++got; });   // busy-poll: never sleeps
        for (std::uint64_t i = 0; i < N; ++i) while (!q.push({0, i})) {}                            // spin while full
    }
    double spsc_s = std::chrono::duration<double>(Clock::now() - t0).count();

    // 2. Latency: one message every 2 microseconds, so the queue is normally empty and we time a single hop
    constexpr std::uint64_t M = 200'000;
    std::vector<std::int64_t> lat; lat.reserve(M);
    {
        std::jthread consumer([&] {
            for (std::uint64_t got = 0; got < M;)
                if (auto m = q.pop()) { lat.push_back(now_ns() - m->sent_ns); ++got; }
        });
        std::int64_t next = now_ns();
        for (std::uint64_t i = 0; i < M; ++i) {
            while (now_ns() < next) {}                                            // pace the sender
            while (!q.push({now_ns(), i})) {}
            next += 2'000;
        }
    }
    std::sort(lat.begin(), lat.end());

    // 3. The same throughput test with a mutex-protected std::queue
    std::queue<Msg> mq; std::mutex m;
    t0 = Clock::now();
    {
        std::jthread consumer([&] {
            for (std::uint64_t got = 0; got < N;) { std::lock_guard g(m); while (!mq.empty()) { mq.pop(); ++got; } }
        });
        for (std::uint64_t i = 0; i < N; ++i) { std::lock_guard g(m); mq.push({0, i}); }
    }
    double mutex_s = std::chrono::duration<double>(Clock::now() - t0).count();

    std::printf("SPSC ring throughput:   %6.1f million msgs/s\n", N / spsc_s / 1e6);
    std::printf("mutex queue throughput: %6.1f million msgs/s\n", N / mutex_s / 1e6);
    std::printf("SPSC one-way latency:   p50 %lld ns, p99 %lld ns, p99.9 %lld ns\n", static_cast<long long>(lat[M / 2]),
                static_cast<long long>(lat[M * 99 / 100]), static_cast<long long>(lat[M * 999 / 1000]));
}
```

Throughput and latency are different questions. In the throughput test the queue is usually full, so a message waits behind a thousand others. That is why latency is measured separately, with paced messages. The latency includes the cost of reading the clock twice. On the shared, virtualised 4-core machine that built this book, the median hop took about half a microsecond, but the 99th and 99.9th percentiles were tens of microseconds and milliseconds: the hypervisor and the OS sometimes pause one of the spinning threads. That is exactly what core isolation and thread pinning remove. On a tuned server with isolated cores, an SPSC hop typically takes under 100 ns, with a short tail.

## False sharing: two threads, one cache line

Caches work in 64-byte lines, and a core must own a line exclusively to write to it. If two threads write **different variables that happen to share a cache line**, the line bounces between the cores on every write, even though there is no logical sharing. This is **false sharing**:

```cpp run flags="-O2"
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
```

The two threads never touch each other's counter, yet padding them apart changes the speed dramatically. `std::hardware_destructive_interference_size` (in `<new>`) gives the cache-line size portably.

## Busy-polling vs sleeping

The consumer above **spins**: it checks the queue in a tight loop and never sleeps. That burns a whole core, but it reacts within nanoseconds. Waiting on a condition variable (chapter 25) saves the CPU but costs microseconds to wake up. Low-latency systems busy-poll on dedicated cores, and add `_mm_pause()` (x86) in the loop to be kinder to the core's other hardware thread. Everything else should sleep.

## Beyond SPSC

| Queue | Use | Difficulty |
|---|---|---|
| SPSC ring (this chapter) | one producer thread to one consumer thread | moderate |
| MPSC (many producers, one consumer) | many threads logging to one writer | harder: producers must claim slots with compare-and-swap |
| MPMC | general work queues | hard; use a proven library (folly's `MPMCQueue`, `moodycamel::ConcurrentQueue`, Boost.Lockfree) |
| Disruptor / broadcast rings | one producer, many consumers each reading everything | the LMAX design, popular in trading |

:::warn Lock-free code is easy to get subtly wrong
Every memory order must be justified, and bugs may show up only on another CPU architecture (ARM reorders more than x86). Test with ThreadSanitizer, stress-test for hours, and prefer well-reviewed libraries for anything beyond SPSC.
:::

::quiz
? Why can an SPSC ring buffer work without locks?
+ Each index is written by only one thread, and release/acquire ordering publishes the data safely
- Atomics are always faster than everything
- It copies all the data
> One writer per variable avoids races; release/acquire provides visibility.
? Two threads update different counters and run slowly. They sit in the same 64-byte block. What is happening?
+ False sharing: the cache line bounces between the cores
- A data race
- A deadlock
> Pad or align the variables onto separate cache lines.
::
