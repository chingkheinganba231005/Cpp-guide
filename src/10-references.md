A **reference** is a second name for an object that already exists. It sounds modest, but references are how C++ passes big objects around without copying them, and how functions modify their callers' data.

## A reference is an alias

```cpp run
#include <iostream>

int main() {
    int score = 10;
    int& alias = score;      // alias IS score: another name for the same 4 bytes
    alias += 5;
    std::cout << score << '\n';                                  // 15
    std::cout << std::boolalpha << (&alias == &score) << '\n';   // same address
}
```

The `&` after a type (`int&`) means “reference to”. The `&` *before* a variable in an expression (`&score`) means “address of”. Same symbol, two meanings. You'll get used to it.

Rules:

- A reference **must be initialised** when it is created: `int& r;` is an error.
- It can never be “re-seated” to refer to something else. `alias = other;` copies `other`'s value *into* `score`.
- There is no “null reference”. A reference always refers to an object.

## Pass by reference: let a function change the caller's variable

```cpp run
#include <iostream>

void add_bonus(int& points) {     // points is another name for the caller's variable
    points += 100;
}

void swap_values(int& a, int& b) {
    int tmp = a;
    a = b;
    b = tmp;
}

int main() {
    int alice = 50, bob = 70;
    add_bonus(alice);
    swap_values(alice, bob);
    std::cout << "alice " << alice << ", bob " << bob << '\n';
}
```

Compare with chapter 5's `try_to_change(int n)`, which received a copy. Adding a single `&` changes the meaning completely.

:::python
In Python, passing a list to a function lets the function mutate it (`xs.append(1)` is visible to the caller), but rebinding (`xs = []`) is not visible. People call this “pass by object reference”. C++ makes the choice explicit: **by value** (a copy: the caller is untouched), **by reference** (`T&`: the function can change the caller's object, even with `=`), or **by const reference** (`const T&`: no copy, and no changes allowed).
:::

## const references: no copy, no changes

Copying a `std::vector` of a million numbers copies a million numbers. A `const&` parameter avoids the copy and also promises not to modify the argument:

```cpp run
#include <chrono>
#include <iostream>
#include <vector>

double sum_by_value(std::vector<double> v) {         // copies the whole vector
    double s = 0; for (double x : v) s += x; return s;
}
double sum_by_ref(const std::vector<double>& v) {    // just refers to the caller's vector
    double s = 0; for (double x : v) s += x; return s;
}

int main() {
    std::vector<double> big(5'000'000, 1.0);
    auto time = [&](auto f) {
        auto t0 = std::chrono::steady_clock::now();
        double s = 0;
        for (int i = 0; i < 20; ++i) s += f(big);
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        return std::pair{s, ms};
    };
    auto [s1, t1] = time(sum_by_value);
    auto [s2, t2] = time(sum_by_ref);
    std::cout << "by value: " << s1 << " in " << t1 << " ms\n";
    std::cout << "by const&: " << s2 << " in " << t2 << " ms\n";
    std::cout << "copying made it about " << static_cast<int>(t1 / t2 + 0.5) << "x slower\n";
}
```

The guideline that every C++ programmer follows:

| Parameter type | Use when |
|---|---|
| `T` (by value) | small, cheap-to-copy types: `int`, `double`, `char`, small structs, `std::string_view`, `std::span` |
| `const T&` | larger types you only read: `std::string`, `std::vector`, your own classes |
| `T&` | the function must modify the caller's object (an “out” parameter) |

## References in range-for loops

The same choice appears in loops. `for (auto x : v)` copies each element; `for (const auto& x : v)` does not; `for (auto& x : v)` lets you modify the elements in place:

```cpp run
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::vector<std::string> names{"ada", "linus", "grace"};
    for (auto& n : names) n[0] = static_cast<char>(n[0] - 'a' + 'A');   // modify in place
    for (const auto& n : names) std::cout << n << ' ';                  // read without copying
    std::cout << '\n';
    for (auto n : names) n = "changed";      // modifies copies only: names is unchanged
    std::cout << names[0] << '\n';
}
```

## Dangling references

A reference must never outlive the object it refers to. Returning a reference to a local variable is the classic mistake:

```cpp run expect=fail flags="-Werror=return-local-addr"
#include <string>

const std::string& make_greeting() {
    std::string s = "hello";
    return s;           // s is destroyed when the function returns: the reference dangles
}

int main() {
    const std::string& g = make_greeting();
    return static_cast<int>(g.size());
}
```

Return by **value** instead (`std::string make_greeting()`). Thanks to *copy elision* and *move semantics* (chapter 15), returning even large objects by value costs nothing extra.

## Under the hood

Most compilers implement a reference as a hidden pointer (an address, chapter 10) that is automatically followed. You can think of it as “a pointer that can't be null, can't be re-seated, and doesn't need `*`”. When a reference refers to a local variable in the same function, the optimiser usually removes it entirely.

:::exercise Normalise in place
Write `void normalise(std::vector<double>& v)` that divides every element by the vector's maximum absolute value, modifying the caller's vector. Then write `double max_abs(const std::vector<double>& v)` and use it. Why does one take `&` and the other `const&`?
---solution---
```cpp run
#include <cmath>
#include <iostream>
#include <vector>

double max_abs(const std::vector<double>& v) {
    double m = 0;
    for (double x : v) m = std::max(m, std::abs(x));
    return m;
}

void normalise(std::vector<double>& v) {
    double m = max_abs(v);
    if (m == 0) return;
    for (double& x : v) x /= m;
}

int main() {
    std::vector<double> v{2, -8, 4};
    normalise(v);
    for (double x : v) std::cout << x << ' ';
    std::cout << '\n';
}
```
`max_abs` only reads, so `const&`. `normalise` modifies the caller's data, so `&`.
:::

::quiz
? After `int a = 1, b = 2; int& r = a; r = b;`, what are a and r?
+ a is 2 and r still refers to a
- r now refers to b
- a is 1
> Assigning through a reference copies the value; a reference cannot be re-seated.
? Which parameter type should a function that prints a large vector use?
- std::vector<int>
- std::vector<int>&
+ const std::vector<int>&
> No copy, and the const documents (and enforces) that it only reads.
::
