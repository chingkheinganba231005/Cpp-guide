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
