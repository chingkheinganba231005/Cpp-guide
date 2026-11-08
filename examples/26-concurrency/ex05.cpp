#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    std::atomic<long long> counter{0};
    {
        std::vector<std::jthread> ts;
        for (int t = 0; t < 4; ++t)
            ts.emplace_back([&] { for (int i = 0; i < 1'000'000; ++i) counter.fetch_add(1, std::memory_order_relaxed); });
    }
    std::cout << "expected 4000000, got " << counter.load() << '\n';
}
