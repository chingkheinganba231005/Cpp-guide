Python's `list` holds any mix of objects and grows freely. Its C++ counterpart is **`std::vector<T>`**: a growable list in which every element has the same type `T`, stored side by side in memory. That “side by side” is why vectors are so fast, and it is the single most important data structure in C++.

## std::vector basics

```cpp run
#include <iostream>
#include <vector>

int main() {
    std::vector<int> scores{90, 72, 85};     // Python: scores = [90, 72, 85]
    scores.push_back(64);                    // append
    std::cout << "size " << scores.size() << ", first " << scores[0] << ", last " << scores.back() << '\n';
    scores[1] = 75;                          // change an element
    int total = 0;
    for (int s : scores) total += s;
    std::cout << "average " << static_cast<double>(total) / scores.size() << '\n';
    scores.pop_back();                       // remove the last element
    scores.insert(scores.begin(), 100);      // insert at the front (slow for big vectors)
    for (int s : scores) std::cout << s << ' ';
    std::cout << '\n';
}
```

| Python list | C++ vector |
|---|---|
| `xs = []` | `std::vector<int> xs;` |
| `xs = [0] * 10` | `std::vector<int> xs(10, 0);` (note the round brackets) |
| `xs.append(x)` | `xs.push_back(x);` |
| `xs.pop()` | `xs.back()` then `xs.pop_back();` |
| `len(xs)` | `xs.size()` |
| `xs[-1]` | `xs.back()` |
| `x in xs` | `std::ranges::find(xs, x) != xs.end()` or `std::ranges::contains(xs, x)` |
| `sorted(xs)` | `std::ranges::sort(xs)` (in place) |
| `xs.clear()` | `xs.clear();` |

:::warn Round vs curly brackets
`std::vector<int> a(3, 7);` is three sevens: `7 7 7`. `std::vector<int> b{3, 7};` is two elements: `3 7`. Curly braces list the elements; round brackets pass a size and a value.
:::

## Indexing is not checked, unless you ask

Python raises `IndexError` for `xs[10]` on a short list. C++'s `v[i]` does **no check**: reading past the end is undefined behaviour, and it may quietly return garbage. `v.at(i)` checks and throws an exception:

```cpp run expect=crash
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{1, 2, 3};
    std::cout << v.at(1) << '\n';
    std::cout << v.at(10) << '\n';     // throws std::out_of_range: the program stops with a message
}
```

Why doesn't `[]` check? Because checking costs a little time on every access, and C++ lets you choose. A common strategy: use `[]` in proven inner loops, and compile your tests with the library's debug checks (`-D_GLIBCXX_DEBUG` for GCC, or AddressSanitizer, chapter 13) so bad indices are caught while developing.

## Sizes are unsigned: a classic trap

`v.size()` returns `std::size_t`, an **unsigned** integer. Subtracting from it can wrap around to a huge number:

```cpp run
#include <iostream>
#include <vector>

int main() {
    std::vector<int> empty;
    std::cout << "empty.size() - 1 = " << empty.size() - 1 << '\n';     // not -1!
    // so `for (std::size_t i = 0; i < v.size() - 1; ++i)` runs (almost) forever on an empty vector
    std::cout << "std::ssize(empty) - 1 = " << std::ssize(empty) - 1 << '\n';   // C++20: a signed size
}
```

Prefer range-for loops, which avoid indices completely. When you need an index, `std::ssize(v)` gives a signed size.

## How a vector grows

A vector keeps its elements in one contiguous block. When it is full, `push_back` allocates a **bigger block** (usually 1.5 to 2 times the size), copies or moves everything across, and frees the old block.

```cpp run
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v;
    std::size_t last_cap = 0;
    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
        if (v.capacity() != last_cap) {
            last_cap = v.capacity();
            std::cout << "size " << v.size() << " -> capacity " << last_cap << '\n';
        }
    }
}
```

Growth is fast on average (“amortised constant time”). When you know the final size, `v.reserve(n)` allocates once up front. In performance-critical code (games, trading, chapter 33), reserving ahead of time avoids allocations at the worst moments.

:::deep
Because the elements move when the vector grows, any pointer or reference to an element becomes invalid after a `push_back` that reallocates. This is a classic C++ bug. Chapter 13 shows how sanitizers catch it.
:::

## std::array: fixed size, no heap

When the size is fixed and known when you write the code, use `std::array<T, N>`. It lives directly inside the function's stack frame or the enclosing object, with no allocation at all:

```cpp run
#include <array>
#include <iostream>

int main() {
    std::array<double, 3> position{1.0, 2.5, -4.0};     // x, y, z
    double len2 = 0;
    for (double c : position) len2 += c * c;
    std::cout << "size " << position.size() << ", length^2 " << len2 << '\n';
}
```

You will also see old **C arrays** (`int a[3] = {1, 2, 3};`). They don't know their own size and convert silently to pointers (chapter 10). Prefer `std::array`.

## 2D data

A vector of vectors works (`std::vector<std::vector<double>> grid(rows, std::vector<double>(cols));`) but scatters the rows across memory. High-performance code (images, matrices, game maps) stores a 2D grid in **one** vector and computes the index:

```cpp run
#include <iostream>
#include <vector>

int main() {
    const int rows = 3, cols = 4;
    std::vector<int> grid(rows * cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            grid[r * cols + c] = r * 10 + c;      // row-major: rows stored one after another
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) std::cout << grid[r * cols + c] << '\t';
        std::cout << '\n';
    }
}
```

This “row-major flat array” layout is exactly how NumPy, images and GPU buffers store data. You will use it in the renderer, the ML library and the matmul chapters.

## Useful algorithms on vectors

```cpp run
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    std::vector<int> v{7, 2, 9, 4, 4, 1};
    std::ranges::sort(v);
    std::cout << "sorted: "; for (int x : v) std::cout << x << ' '; std::cout << '\n';
    std::cout << "sum " << std::accumulate(v.begin(), v.end(), 0)
              << ", max " << std::ranges::max(v)
              << ", count of 4: " << std::ranges::count(v, 4) << '\n';
    std::ranges::reverse(v);
    auto it = std::ranges::find(v, 9);
    std::cout << "9 found at index " << (it - v.begin()) << '\n';
}
```

Chapter 19 covers the algorithms library properly.

:::exercise Running statistics
Read numbers from input until it ends, store them in a vector, then print the count, mean, minimum, maximum and median (sort a copy; the median of an even count is the average of the two middle values).
---solution---
```cpp run stdin="4 8 15 16 23 42"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    std::vector<double> xs;
    for (double x; std::cin >> x;) xs.push_back(x);
    if (xs.empty()) return 0;
    auto sorted = xs;
    std::ranges::sort(sorted);
    double mean = std::accumulate(xs.begin(), xs.end(), 0.0) / xs.size();
    std::size_t n = sorted.size();
    double median = n % 2 ? sorted[n / 2] : (sorted[n / 2 - 1] + sorted[n / 2]) / 2;
    std::cout << "n=" << n << " mean=" << mean << " min=" << sorted.front() << " max=" << sorted.back() << " median=" << median << '\n';
}
```
:::

::quiz
? `std::vector<int> v(5);` contains…
+ five zeros
- one element, 5
- nothing
> Round brackets give a size; elements are value-initialised to 0.
? What does `v[100]` do on a 3-element vector?
- Throws an exception
+ Undefined behaviour: no check is made
- Returns 0
> Use v.at(100) for a checked access that throws.
? Why does high-performance code store a 2D grid in one vector?
+ The data is contiguous, which is faster and simpler
- Vectors of vectors are not allowed
- It uses less typing
> One block of memory means fewer allocations and cache-friendly access.
::
