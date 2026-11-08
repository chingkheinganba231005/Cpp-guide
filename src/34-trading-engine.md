An exchange's **matching engine** is the program at the centre of every market: it receives buy and sell orders, keeps them in an **order book**, and executes trades when prices cross. Market makers and high-frequency trading firms run similar code inside their own systems, where reacting a microsecond faster than the competition is the whole business. In this project you build a matching engine twice, a straightforward one and an optimised one, and measure the latency of each.

## How an order book works

- A **limit order** says “buy 30 at 101 or better”. If it can't trade immediately, it **rests** in the book.
- A **market order** says “buy 30 at whatever price”. It trades against the best resting orders right away.
- The book has two sides: **bids** (buyers, highest price first) and **asks** (sellers, lowest price first). The gap between the best bid and the best ask is the **spread**.
- Matching follows **price-time priority**: the best price trades first, and at the same price, the order that arrived first trades first (a FIFO queue per price level).
- Orders can be **cancelled** at any time.

Try it: send a buy limit at 101 for 30 and watch it consume order #5, then part of #6. Send a sell limit at 100 and watch it rest. Send a market buy for 200 and watch it sweep several levels.

<div data-widget="orderbook" data-title="An interactive order book (price-time priority)"></div>

## The interface

Both engines implement the same operations and report trades through a callback:

```cpp file=book_common.hpp
#pragma once
#include <cstdint>

enum class Side : std::uint8_t { Buy, Sell };

struct Trade { std::uint64_t taker, maker; std::int32_t price, qty; };

struct Stats {                                          // a running summary, used to check both engines agree
    std::uint64_t trades = 0, volume = 0, notional = 0;
    void on_trade(const Trade& t) { ++trades; volume += t.qty; notional += static_cast<std::uint64_t>(t.price) * t.qty; }
};
```

## Version 1: clear and correct (std::map + std::list)

```cpp file=simple_book.hpp
#pragma once
#include <functional>
#include <list>
#include <map>
#include <unordered_map>
#include "book_common.hpp"

class SimpleBook {
    struct Order { std::uint64_t id; std::int32_t qty; };
    using Level = std::list<Order>;                                    // FIFO queue at one price
    std::map<std::int32_t, Level, std::greater<>> bids_;              // highest price first
    std::map<std::int32_t, Level> asks_;                               // lowest price first
    struct Loc { Side side; std::int32_t price; Level::iterator it; };
    std::unordered_map<std::uint64_t, Loc> index_;                     // id -> where the order rests (for cancels)
public:
    Stats stats;

    void limit(std::uint64_t id, Side side, std::int32_t price, std::int32_t qty) {
        qty = side == Side::Buy ? match(asks_, id, qty, [&](std::int32_t p) { return p <= price; })
                                : match(bids_, id, qty, [&](std::int32_t p) { return p >= price; });
        if (qty > 0) {                                                 // rest the remainder
            if (side == Side::Buy) { auto& lv = bids_[price]; lv.push_back({id, qty}); index_[id] = {side, price, std::prev(lv.end())}; }
            else                   { auto& lv = asks_[price]; lv.push_back({id, qty}); index_[id] = {side, price, std::prev(lv.end())}; }
        }
    }
    void market(std::uint64_t id, Side side, std::int32_t qty) {
        if (side == Side::Buy) match(asks_, id, qty, [](std::int32_t) { return true; });
        else                   match(bids_, id, qty, [](std::int32_t) { return true; });
    }
    void cancel(std::uint64_t id) {
        auto f = index_.find(id);
        if (f == index_.end()) return;                                 // already filled or cancelled
        auto [side, price, it] = f->second;
        if (side == Side::Buy) erase_from(bids_, price, it); else erase_from(asks_, price, it);
        index_.erase(f);
    }
private:
    template <typename Book, typename Crosses>
    std::int32_t match(Book& book, std::uint64_t taker, std::int32_t qty, Crosses crosses) {
        while (qty > 0 && !book.empty() && crosses(book.begin()->first)) {
            auto& [price, level] = *book.begin();
            Order& maker = level.front();
            std::int32_t q = std::min(qty, maker.qty);
            stats.on_trade({taker, maker.id, price, q});
            qty -= q; maker.qty -= q;
            if (maker.qty == 0) { index_.erase(maker.id); level.pop_front(); }
            if (level.empty()) book.erase(book.begin());
        }
        return qty;
    }
    template <typename Book>
    void erase_from(Book& book, std::int32_t price, Level::iterator it) {
        auto lv = book.find(price);
        lv->second.erase(it);
        if (lv->second.empty()) book.erase(lv);
    }
};
```

This is a perfectly reasonable design, and it is what most people write first. Every new resting order allocates a list node, and often a map node and a hash-table node too. Every lookup chases pointers through a tree scattered across the heap.

## Version 2: built for latency

Professional engines remove every source of unpredictable delay:

- **No heap allocation after start-up**: orders come from a pre-allocated **pool** (chapter 27).
- **Price levels in a flat array** indexed by price (prices are integer *ticks* within a known range), so finding a level is one array access.
- **Intrusive FIFO lists**: each order carries its own `prev`/`next` pointers, so a level is just a head and a tail pointer. Cancels unlink in O(1).
- **Order lookup in a flat vector** indexed by order ID (exchanges assign dense IDs) instead of a hash map.
- **Track the best bid and ask** so matching starts in the right place.

```cpp file=fast_book.hpp
#pragma once
#include <cstdint>
#include <vector>
#include "book_common.hpp"

class FastBook {
    struct Order { std::uint64_t id; std::int32_t price, qty; Side side; Order *prev = nullptr, *next = nullptr; };
    struct Level { Order *head = nullptr, *tail = nullptr; };
public:
    Stats stats;

    FastBook(std::int32_t max_price, std::size_t max_orders, std::size_t max_ids)
        : bids_(max_price + 1), asks_(max_price + 1), pool_(max_orders), by_id_(max_ids, nullptr), max_price_(max_price) {
        for (std::size_t i = 0; i + 1 < pool_.size(); ++i) pool_[i].next = &pool_[i + 1];   // build the free list
        free_ = pool_.empty() ? nullptr : &pool_[0];
        best_ask_ = max_price + 1; best_bid_ = -1;
    }

    void limit(std::uint64_t id, Side side, std::int32_t price, std::int32_t qty) {
        qty = (side == Side::Buy) ? match_buy(id, qty, price) : match_sell(id, qty, price);
        if (qty > 0) rest(id, side, price, qty);
    }
    void market(std::uint64_t id, Side side, std::int32_t qty) {
        if (side == Side::Buy) match_buy(id, qty, max_price_); else match_sell(id, qty, 0);
    }
    void cancel(std::uint64_t id) {
        Order* o = id < by_id_.size() ? by_id_[id] : nullptr;
        if (!o) return;
        Level& lv = (o->side == Side::Buy ? bids_ : asks_)[o->price];
        unlink(lv, o);
        if (!lv.head) {                                               // the level emptied: maybe move the best price
            if (o->side == Side::Buy && o->price == best_bid_) while (best_bid_ >= 0 && !bids_[best_bid_].head) --best_bid_;
            if (o->side == Side::Sell && o->price == best_ask_) while (best_ask_ <= max_price_ && !asks_[best_ask_].head) ++best_ask_;
        }
        release(o);
    }

private:
    std::int32_t match_buy(std::uint64_t taker, std::int32_t qty, std::int32_t limit) {
        while (qty > 0 && best_ask_ <= limit) {
            Level& lv = asks_[best_ask_];
            qty = fill_level(lv, taker, qty, best_ask_);
            if (!lv.head) while (best_ask_ <= max_price_ && !asks_[best_ask_].head) ++best_ask_;
        }
        return qty;
    }
    std::int32_t match_sell(std::uint64_t taker, std::int32_t qty, std::int32_t limit) {
        while (qty > 0 && best_bid_ >= limit && best_bid_ >= 0) {
            Level& lv = bids_[best_bid_];
            qty = fill_level(lv, taker, qty, best_bid_);
            if (!lv.head) while (best_bid_ >= 0 && !bids_[best_bid_].head) --best_bid_;
        }
        return qty;
    }
    std::int32_t fill_level(Level& lv, std::uint64_t taker, std::int32_t qty, std::int32_t price) {
        while (qty > 0 && lv.head) {
            Order* m = lv.head;
            std::int32_t q = qty < m->qty ? qty : m->qty;
            stats.on_trade({taker, m->id, price, q});
            qty -= q; m->qty -= q;
            if (m->qty == 0) { unlink(lv, m); release(m); }
        }
        return qty;
    }
    void rest(std::uint64_t id, Side side, std::int32_t price, std::int32_t qty) {
        Order* o = free_;                                             // pop from the pool: no malloc
        free_ = o->next;
        *o = Order{id, price, qty, side, nullptr, nullptr};
        Level& lv = (side == Side::Buy ? bids_ : asks_)[price];
        o->prev = lv.tail;                                            // append at the back: time priority
        if (lv.tail) lv.tail->next = o; else lv.head = o;
        lv.tail = o;
        by_id_[id] = o;
        if (side == Side::Buy && price > best_bid_) best_bid_ = price;
        if (side == Side::Sell && price < best_ask_) best_ask_ = price;
    }
    static void unlink(Level& lv, Order* o) {
        if (o->prev) o->prev->next = o->next; else lv.head = o->next;
        if (o->next) o->next->prev = o->prev; else lv.tail = o->prev;
    }
    void release(Order* o) { by_id_[o->id] = nullptr; o->next = free_; free_ = o; }

    std::vector<Level> bids_, asks_;
    std::vector<Order> pool_;
    std::vector<Order*> by_id_;
    Order* free_ = nullptr;
    std::int32_t max_price_, best_bid_, best_ask_;
};
```

## The benchmark: identical results, very different latency

Both engines receive the same 2 million messages: 60% limit orders clustered around a drifting mid price, 10% market orders and 30% cancels. We check that they produce exactly the same trades, then measure the time to process each message and report **percentiles**. In trading, the slow outliers (the 99th and 99.9th percentiles, the “tail latency”) matter as much as the median, because that is where you lose.

```cpp run with=book_common.hpp,simple_book.hpp,fast_book.hpp flags="-O3 -march=native"
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
```

Reading the results: both engines produce exactly the same trades. The flat, pool-based engine roughly halves the median and the 99th and 99.9th percentiles, simply by avoiding allocation and pointer chasing. Look at the **max** column too. Both engines occasionally take about a millisecond, thousands of times longer than usual. That isn't the engine's code: it is the operating system interrupting the thread (a timer interrupt, another process scheduled on the same core, the kernel doing housekeeping). No amount of clever C++ fixes that alone, which is why real trading systems also pin their threads to isolated CPU cores (see the table below).

(The timer itself costs a few tens of nanoseconds per reading, so the smallest numbers are dominated by measurement overhead. Production systems timestamp with the CPU's cycle counter, `rdtsc`, or with hardware timestamps on the network card.)

## The rest of a trading system

| Component | What it does | Key C++ techniques |
|---|---|---|
| **Market-data feed handler** | parses exchange binary messages (e.g. ITCH, SBE) at millions per second | zero-copy parsing with `std::span`, `std::from_chars`, packed structs, `static_assert` on layouts |
| **Order book(s)** | this chapter, for every instrument | flat arrays, pools, cache-friendly layouts |
| **Strategy** | decides what to send, in microseconds | no allocation, no exceptions, branch-light code, precomputed tables |
| **Risk checks** | position, price and rate limits before anything leaves | constant-time checks, fail-safe defaults |
| **Order gateway** | sends orders to exchanges (FIX, native binary protocols) | kernel-bypass networking (DPDK, Solarflare/OpenOnload), busy-polling |
| **Messaging between threads** | moves data between pinned cores | lock-free single-producer single-consumer queues (chapter 34) |
| **Logging** | records everything without slowing the hot path | write binary records to a ring buffer; a background thread formats them |

System-level tuning matters as much as code: threads pinned to isolated CPU cores, no context switches, huge pages, NUMA-aware memory, and busy-polling instead of sleeping. The C++ principle underneath is always the same: **do the work you must, ahead of time, in predictable memory, and nothing else on the hot path.**

:::exercise Make the engine more realistic
1. Add an *immediate-or-cancel* (IOC) order type: it matches like a limit order but never rests.
2. Add a `modify(id, new_qty)` operation. Decreasing the quantity keeps the order's time priority; increasing it moves the order to the back of the queue (that is the usual exchange rule).
3. Record the latency of only the *limit orders that traded* and compare their percentiles with those of cancels. Which operation is most expensive in each engine, and why?
---solution---
For IOC, call the matching function and simply don't call `rest()` with the remainder. For modify: if `new_qty < qty`, change the quantity in place. Otherwise `unlink` the order, update it, and append it at the tail of its level. In `SimpleBook` an aggressive limit order that sweeps several levels erases map nodes and frees list nodes, which is expensive. In `FastBook` its cost is mostly the walk over levels and the per-trade callback.
:::

::quiz
? With price-time priority, which resting sell order trades first against an incoming buy?
+ The lowest-priced one; among equal prices, the oldest
- The largest one
- The newest one
> Best price first, then first come, first served.
? Why does the fast engine avoid std::map and new on its hot path?
+ Heap allocation and pointer-chasing through a tree add unpredictable latency
- std::map is incorrect
- new is not allowed in C++
> Pools and flat arrays make every operation short and predictable.
? Why do trading systems report p99 and p99.9 latency, not just the average?
+ Rare slow events are where opportunities are lost; averages hide them
- Averages are hard to compute
- Exchanges require it
> Tail latency decides who wins the race when it matters.
::
