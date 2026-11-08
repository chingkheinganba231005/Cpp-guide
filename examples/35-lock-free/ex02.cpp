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
