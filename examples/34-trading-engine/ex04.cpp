#include <algorithm>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>
#include "fast_book.hpp"
#include "simple_book.hpp"

struct Msg { enum Kind : std::uint8_t { Limit, Market, Cancel } kind; Side side; std::int32_t price, qty; std::uint64_t id; };

std::vector<Msg> make_flow(std::size_t n) {
    std::mt19937_64 rng(2024);
    std::vector<Msg> out; out.reserve(n);
    std::vector<std::uint64_t> live;                       // ids that may still be resting
    double mid = 10'000;
    for (std::uint64_t id = 1; out.size() < n; ++id) {
        mid += std::normal_distribution<double>(0, 0.3)(rng);
        double u = std::uniform_real_distribution<double>(0, 1)(rng);
        Side side = rng() % 2 ? Side::Buy : Side::Sell;
        std::int32_t qty = 1 + static_cast<std::int32_t>(rng() % 100);
        if (u < 0.30 && !live.empty()) {
            std::size_t k = rng() % live.size();
            out.push_back({Msg::Cancel, side, 0, 0, live[k]});
            live[k] = live.back(); live.pop_back();
        } else if (u < 0.40) {
            out.push_back({Msg::Market, side, 0, qty, id});
        } else {
            int offset = static_cast<int>(std::abs(std::normal_distribution<double>(0, 8)(rng)));     // passive orders, ticks away from mid
            std::int32_t px = static_cast<std::int32_t>(mid) + (side == Side::Buy ? -offset : offset) + (rng() % 10 == 0 ? (side == Side::Buy ? 3 : -3) : 0);
            out.push_back({Msg::Limit, side, px, qty, id});
            live.push_back(id);
        }
    }
    return out;
}

template <typename Book>
void run(const char* name, Book& book, const std::vector<Msg>& flow) {
    std::vector<std::uint32_t> ns(flow.size());
    for (std::size_t i = 0; i < flow.size(); ++i) {
        const Msg& m = flow[i];
        auto t0 = std::chrono::steady_clock::now();
        switch (m.kind) {
            case Msg::Limit: book.limit(m.id, m.side, m.price, m.qty); break;
            case Msg::Market: book.market(m.id, m.side, m.qty); break;
            case Msg::Cancel: book.cancel(m.id); break;
        }
        ns[i] = static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0).count());
    }
    std::sort(ns.begin(), ns.end());
    auto pct = [&](double p) { return ns[static_cast<std::size_t>(p * (ns.size() - 1))]; };
    std::printf("%-12s p50 %4u ns   p99 %5u ns   p99.9 %6u ns   max %8u ns   | trades %llu, volume %llu\n", name, pct(0.5), pct(0.99), pct(0.999), ns.back(),
                static_cast<unsigned long long>(book.stats.trades), static_cast<unsigned long long>(book.stats.volume));
}

int main() {
    const std::size_t N = 2'000'000;
    auto flow = make_flow(N);
    for (int round = 0; round < 2; ++round) {             // round 0 warms up caches and the allocator; we report round 1
        SimpleBook simple;
        FastBook fast(20'000, N, N + 2);
        if (round == 0) { run("(warm-up)", simple, flow); continue; }
        run("std::map", simple, flow);
        run("flat+pool", fast, flow);
        std::printf("identical results: %s\n", simple.stats.trades == fast.stats.trades && simple.stats.notional == fast.stats.notional ? "yes" : "NO");
    }
}
