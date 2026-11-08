The C++ **standard library** gives you battle-tested containers and around a hundred algorithms. Knowing which container to choose, and what each operation costs, is one of the biggest differences between beginner and expert C++. This chapter is your map.

## Big-O in one minute

**Big-O** describes how the work grows as the data grows:

| Notation | Name | Example | 1,000 items → 1,000,000 items |
|---|---|---|---|
| O(1) | constant | `v[i]`, hash-map lookup (average) | same time |
| O(log n) | logarithmic | `std::map` lookup, binary search | ~2× longer |
| O(n) | linear | scanning a vector | 1000× longer |
| O(n log n) | | sorting | ~2000× longer |
| O(n²) | quadratic | comparing every pair | 1,000,000× longer |

Big-O ignores constant factors, and in real programs constant factors are often decided by the **CPU cache** (chapter 26). That is why `std::vector` usually beats “theoretically better” containers for small and medium sizes.

## The containers

| Container | Python analogue | Structure | Use for |
|---|---|---|---|
| `std::vector<T>` | `list` | contiguous dynamic array | **the default for almost everything** |
| `std::array<T, N>` | tuple of fixed length | fixed contiguous array | small fixed-size data (coordinates, matrices) |
| `std::deque<T>` | `collections.deque` | blocks of arrays | fast push/pop at both ends |
| `std::list<T>` | — | doubly linked list | rarely; stable references with frequent middle insertions |
| `std::unordered_map<K, V>` | `dict` | hash table | lookup by key, O(1) average |
| `std::map<K, V>` | `dict` kept sorted | balanced tree (red-black) | sorted keys, ranges, order books (chapter 33) |
| `std::unordered_set<T>` / `std::set<T>` | `set` / sorted set | hash / tree | membership tests, unique items |
| `std::priority_queue<T>` | `heapq` | binary heap on a vector | always get the largest (or smallest) next: schedulers, Dijkstra |
| `std::string` | `str` | contiguous chars | text |

## Maps: dictionaries

```cpp run
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

int main() {
    std::string text = "the cat and the dog and the bird";
    std::unordered_map<std::string, int> counts;      // a hash map: Python dict
    std::istringstream in(text);
    for (std::string w; in >> w;) ++counts[w];        // [] creates the entry (with 0) if missing
    std::cout << "the: " << counts["the"] << ", and: " << counts["and"] << '\n';

    std::map<std::string, int> sorted(counts.begin(), counts.end());   // a tree map: keys come out sorted
    for (const auto& [word, n] : sorted) std::cout << word << '=' << n << ' ';
    std::cout << '\n';

    if (auto it = sorted.find("cat"); it != sorted.end()) std::cout << "found cat: " << it->second << '\n';
    std::cout << std::boolalpha << sorted.contains("fish") << '\n';   // C++20
}
```

:::warn `[]` on a map inserts
`counts["fish"]` *creates* a `"fish"` entry with value 0 if it doesn't exist. To look something up without changing the map, use `find`, `contains` or `at`. `at` throws if the key is missing, like Python's `d["x"]` raising `KeyError`.
:::

`std::map` keeps keys sorted, so you can ask for “the first key ≥ 100” with `lower_bound`. That is exactly what an order book needs (chapter 33). `std::unordered_map` is faster for plain lookups but has no order.

## Iterators: the glue between containers and algorithms

An **iterator** is a generalised pointer into a container. `c.begin()` points to the first element and `c.end()` points **one past the last**, so a range is `[begin, end)`. Algorithms work on iterator ranges, which is why one `std::sort` works for vectors, arrays and deques alike.

```cpp run
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{10, 20, 30};
    for (auto it = v.begin(); it != v.end(); ++it) std::cout << *it << ' ';   // what range-for does for you
    std::cout << '\n' << "distance end - begin = " << (v.end() - v.begin()) << '\n';
}
```

## Algorithms

The `<algorithm>` and `<numeric>` headers hold ready-made, heavily optimised loops. Using them makes code shorter and clearer. C++20 added **ranges** versions (`std::ranges::sort(v)`) that take the whole container:

```cpp run
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct Trade { std::string symbol; double price; int qty; };

int main() {
    std::vector<Trade> trades{{"AAPL", 189.5, 100}, {"MSFT", 410.2, 50}, {"AAPL", 190.1, 200}, {"NVDA", 120.7, 300}};

    std::ranges::sort(trades, {}, &Trade::price);                        // sort by price (a "projection")
    for (const auto& t : trades) std::cout << t.symbol << '@' << t.price << ' ';
    std::cout << '\n';

    auto big = std::ranges::count_if(trades, [](const Trade& t) { return t.qty >= 200; });
    double notional = std::accumulate(trades.begin(), trades.end(), 0.0,
                                      [](double acc, const Trade& t) { return acc + t.price * t.qty; });
    auto most = std::ranges::max_element(trades, {}, &Trade::qty);
    std::cout << big << " big trades, notional " << notional << ", largest qty " << most->symbol << '\n';

    std::erase_if(trades, [](const Trade& t) { return t.symbol == "AAPL"; });    // remove matching elements (C++20)
    std::cout << trades.size() << " left\n";

    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 1);                                        // 1..10
    std::vector<int> squares;
    std::ranges::transform(v, std::back_inserter(squares), [](int x) { return x * x; });   // Python: [x*x for x in v]
    std::cout << "binary search for 49: " << std::boolalpha << std::ranges::binary_search(squares, 49) << '\n';
}
```

The most useful algorithms: `sort`, `stable_sort`, `find`, `find_if`, `count_if`, `any_of`/`all_of`/`none_of`, `transform`, `accumulate`/`reduce`, `min_element`/`max_element`, `binary_search`, `lower_bound`, `unique`, `reverse`, `rotate`, `partition`, `nth_element` (an O(n) median), `copy`, `fill` and `iota`.

## Lambdas in depth: captures

```cpp run
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

int main() {
    int limit = 5, hits = 0;
    std::vector<int> v{1, 7, 3, 9, 5};

    auto over = [limit](int x) { return x > limit; };       // capture limit by value (a copy)
    auto count = [&hits](int x) { if (x % 2) ++hits; };     // capture hits by reference
    std::ranges::for_each(v, count);
    std::cout << std::ranges::count_if(v, over) << " over " << limit << ", " << hits << " odd\n";

    auto make_adder = [](int n) { return [n](int x) { return x + n; }; };   // a lambda returning a lambda
    std::function<int(int)> add10 = make_adder(10);          // std::function can hold any callable
    std::cout << add10(5) << '\n';

    int counter = 0;
    auto tick = [counter]() mutable { return ++counter; };   // mutable: may modify its own copy
    std::cout << tick() << tick() << tick() << " (outer counter is still " << counter << ")\n";
}
```

| Capture | Meaning |
|---|---|
| `[x]` | copy x into the lambda |
| `[&x]` | refer to the outside x (it must outlive the lambda!) |
| `[=]` / `[&]` | capture everything used, by copy / by reference |
| `[this]` | access the current object's members |
| `[p = std::move(ptr)]` | move something into the lambda |

`std::function` can hold any callable with a matching signature. It is convenient for callbacks, but it may allocate and adds an indirect call. Use templates or `auto` when the extra speed matters.

## Choosing a container: the practical rules

1. **Start with `std::vector`.** Even for searching: a linear scan through 100 contiguous elements beats a tree.
2. Need lookup by key: **`std::unordered_map`**. Need sorted keys or range queries: **`std::map`**.
3. A queue or stack: `std::deque`, or `std::vector` with `push_back`/`pop_back`.
4. Avoid `std::list` unless you have measured a reason.
5. In performance-critical code, reserve capacity, avoid maps of strings in hot loops, and consider flat structures such as a sorted vector (`std::flat_map` arrives in C++23 library implementations).

```cpp run
#include <chrono>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>

template <typename C>
double time_sum(const C& c) {
    auto t0 = std::chrono::steady_clock::now();
    long long s = std::accumulate(c.begin(), c.end(), 0LL);
    auto t1 = std::chrono::steady_clock::now();
    if (s == 42) std::cout << "";                    // use the result
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    const int n = 5'000'000;
    std::vector<int> v(n, 1);
    std::list<int> l(v.begin(), v.end());
    double tv = time_sum(v), tl = time_sum(l);
    std::cout << "vector: " << tv << " ms, list: " << tl << " ms (" << static_cast<int>(tl / tv) << "x slower)\n";
}
```

Both are O(n), yet the list is far slower: its nodes are scattered across memory and every step is a pointer chase, which is chapter 26's story.

::quiz
? Which container should you reach for first?
+ std::vector
- std::list
- std::map
> Contiguous memory makes vector the fastest general-purpose container.
? What does `counts["new"]` do on a std::map that has no "new" key?
+ Inserts "new" with a default value (0) and returns a reference to it
- Throws std::out_of_range
- Returns nullptr
> Use find(), contains() or at() to look without inserting.
? A lambda captures `[&total]` and is stored and called after total's function has returned. What is the problem?
+ The reference dangles: total no longer exists
- Nothing
- The lambda copies total
> Reference captures must not outlive the variable.
::
