**Operator overloading** lets your own types use `+`, `==`, `<`, `[]` and `<<` just like built-in types. Done well, it makes maths code (vectors, matrices, money, complex numbers) read like the formulas it implements. This is how `std::string` supports `+` and how linear-algebra libraries such as Eigen and GLM feel natural.

:::python
Python has the same idea with “dunder” methods: `__add__`, `__eq__`, `__lt__`, `__getitem__`, `__str__`. C++ names them `operator+`, `operator==`, `operator<`, `operator[]`, and `operator<<` for printing.
:::

## A vector type for games and physics

```cpp run
#include <cmath>
#include <iostream>

struct Vec2 {
    double x = 0, y = 0;

    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }   // compound assignment modifies *this
    Vec2& operator*=(double k) { x *= k; y *= k; return *this; }
    double length() const { return std::hypot(x, y); }
};

// Binary operators as free functions, built on the compound ones
inline Vec2 operator+(Vec2 a, const Vec2& b) { return a += b; }
inline Vec2 operator-(const Vec2& a, const Vec2& b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(Vec2 v, double k) { return v *= k; }
inline Vec2 operator*(double k, Vec2 v) { return v *= k; }        // so 2 * v works too
inline Vec2 operator-(const Vec2& v) { return {-v.x, -v.y}; }     // unary minus
inline double dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }

std::ostream& operator<<(std::ostream& os, const Vec2& v) {      // printing
    return os << '(' << v.x << ", " << v.y << ')';
}

int main() {
    Vec2 pos{1, 2}, vel{0.5, -1}, gravity{0, -9.81};
    double dt = 0.1;
    vel += gravity * dt;              // reads like physics
    pos += vel * dt;
    std::cout << "pos " << pos << ", vel " << vel << ", speed " << vel.length() << '\n';
    std::cout << "dot " << dot(Vec2{1, 0}, Vec2{0, 1}) << ", -pos " << -pos << ", 2*pos " << 2 * pos << '\n';
}
```

The conventions:

- `+=`, `-=` and `*=` modify the object and return `*this` by reference.
- `+`, `-` and `*` return a new value. Write them as free functions using the compound version, so the logic lives in one place.
- `<<` for printing takes `std::ostream&` and returns it, so calls can chain.

## Comparisons: == and the spaceship <=>

C++20 makes comparisons easy. `= default` generates them member by member, and `<=>` (the “spaceship operator”) generates all of `<`, `<=`, `>` and `>=` at once:

```cpp run
#include <algorithm>
#include <compare>
#include <iostream>
#include <string>
#include <vector>

struct Version {
    int major = 0, minor = 0, patch = 0;
    auto operator<=>(const Version&) const = default;   // compares major, then minor, then patch
    bool operator==(const Version&) const = default;
};

int main() {
    std::vector<Version> vs{{1, 2, 3}, {1, 10, 0}, {0, 9, 9}, {1, 2, 0}};
    std::ranges::sort(vs);
    for (const auto& v : vs) std::cout << v.major << '.' << v.minor << '.' << v.patch << "  ";
    std::cout << '\n' << std::boolalpha << (Version{1, 2, 3} < Version{1, 10, 0}) << ' ' << (Version{1, 0, 0} == Version{1, 0, 0}) << '\n';
}
```

Being comparable is what lets your types go into `std::ranges::sort`, `std::map` and `std::set` (chapter 19).

## Indexing and calling: [] and ()

```cpp run
#include <cstddef>
#include <iostream>
#include <vector>

class Matrix {
public:
    Matrix(std::size_t r, std::size_t c) : rows_(r), cols_(c), data_(r * c, 0.0) {}
    double& operator()(std::size_t r, std::size_t c) { return data_[r * cols_ + c]; }              // m(r, c) = ...
    double operator()(std::size_t r, std::size_t c) const { return data_[r * cols_ + c]; }         // read-only version
    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }
private:
    std::size_t rows_, cols_;
    std::vector<double> data_;     // row-major, as in chapter 7
};

Matrix operator*(const Matrix& a, const Matrix& b) {
    Matrix c(a.rows(), b.cols());
    for (std::size_t i = 0; i < a.rows(); ++i)
        for (std::size_t k = 0; k < a.cols(); ++k)
            for (std::size_t j = 0; j < b.cols(); ++j)
                c(i, j) += a(i, k) * b(k, j);
    return c;
}

int main() {
    Matrix a(2, 2), b(2, 2);
    a(0, 0) = 1; a(0, 1) = 2; a(1, 0) = 3; a(1, 1) = 4;
    b(0, 0) = 0; b(0, 1) = 1; b(1, 0) = 1; b(1, 1) = 0;
    Matrix c = a * b;
    std::cout << c(0, 0) << ' ' << c(0, 1) << '\n' << c(1, 0) << ' ' << c(1, 1) << '\n';
}
```

Two versions of `operator()`: a non-const one that returns a reference (so you can assign through it) and a const one for reading. C++23 also allows `m[r, c]` with a multi-argument `operator[]`. Chapter 31 makes this exact multiplication loop about 100× faster.

## A class you can call: function objects

Overloading `operator()` with no other purpose makes an object callable like a function. It can carry state, and this is exactly what a lambda is under the hood:

```cpp run
#include <iostream>

class Counter {
public:
    int operator()() { return ++count_; }
private:
    int count_ = 0;
};

int main() {
    Counter next_id;
    std::cout << next_id() << ' ' << next_id() << ' ' << next_id() << '\n';
}
```

## Conversions

`explicit operator bool() const` lets an object be tested in `if (obj)` (as `unique_ptr` and streams do) without allowing accidental arithmetic. Avoid non-explicit conversion operators: silent conversions make overloaded code surprising.

## When not to overload

Overload an operator only when its meaning is **obvious and conventional**: `+` on vectors, `==` on values, `[]` on containers, `<<` for printing. Don't use `+` to mean “add to a database” or `*` to mean “format”. If a reader has to look up what an operator does, a named function is better.

:::exercise Money
Write `struct Money { long long cents; };` with `+`, `-`, `*` by an integer, `<=>` and `==` (defaulted), and `<<` that prints `$12.34`. Why store cents as an integer instead of a `double`?
---solution---
```cpp run
#include <compare>
#include <format>
#include <iostream>

struct Money {
    long long cents = 0;
    auto operator<=>(const Money&) const = default;
    bool operator==(const Money&) const = default;
};
Money operator+(Money a, Money b) { return {a.cents + b.cents}; }
Money operator-(Money a, Money b) { return {a.cents - b.cents}; }
Money operator*(Money a, long long k) { return {a.cents * k}; }
std::ostream& operator<<(std::ostream& os, Money m) {
    long long a = m.cents < 0 ? -m.cents : m.cents;
    return os << (m.cents < 0 ? "-" : "") << '$' << a / 100 << '.' << std::format("{:02}", a % 100);
}

int main() {
    Money price{1999}, tax{160};
    Money total = (price + tax) * 3;
    std::cout << total << ' ' << std::boolalpha << (price > tax) << '\n';
}
```
Integer cents are exact; doubles cannot represent 0.10 exactly (chapter 2), so totals drift. Trading systems store prices as integer ticks for the same reason (chapter 33).
:::

::quiz
? Why does `operator+=` return `*this` by reference?
+ So that it modifies the object in place and can be chained, like built-in types
- It is required by the compiler
- To make a copy
> `a += b += c` works for ints, so it should work for your types too.
? What does `auto operator<=>(const T&) const = default;` give you?
+ <, <=, > and >= comparing members in declaration order
- Only ==
- Printing
> Add `bool operator==(const T&) const = default;` for equality.
::
