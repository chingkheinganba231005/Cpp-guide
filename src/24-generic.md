Chapter 18 introduced templates. This chapter covers the tools that library authors use: **concepts** to state requirements clearly, **variadic templates** for any number of arguments, **type traits** to inspect types at compile time, and **CRTP** for zero-cost static polymorphism. You will see these inside every serious library: Eigen, PyTorch's ATen, game engines and trading frameworks.

## Concepts: say what a template needs

A **concept** is a named set of requirements on a type. Put it in the template's signature, and the compiler checks it at the call site:

```cpp run
#include <concepts>
#include <iostream>
#include <string>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>                                 // T must satisfy Numeric
T square(T x) { return x * x; }

template <typename T>
concept Shape = requires(const T& s) {               // a custom concept: "has an area() returning something numeric"
    { s.area() } -> std::convertible_to<double>;
};

struct Square { double side; double area() const { return side * side; } };

double total_area(const Shape auto& a, const Shape auto& b) { return a.area() + b.area(); }

int main() {
    std::cout << square(7) << ' ' << square(1.5) << '\n';
    std::cout << total_area(Square{2}, Square{3}) << '\n';
}
```

Now misuse produces a clear message at the call site:

```cpp run expect=fail
#include <concepts>
#include <string>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>
T square(T x) { return x * x; }

int main() {
    square(std::string("hi"));
}
```

GCC reports `no matching function for call to 'square(std::string)'`, shows the candidate with `requires Numeric<T>`, and explains in its notes that the constraints are not satisfied. All of it points at **your** call. Compare that with chapter 18's error from deep inside the template body. Common standard concepts: `std::integral`, `std::floating_point`, `std::same_as`, `std::convertible_to`, `std::invocable`, `std::totally_ordered`, `std::ranges::range`, `std::copyable`.

## Variadic templates and fold expressions

`typename... Args` accepts any number of types. A **fold expression** applies an operator across all of them:

```cpp run
#include <iostream>
#include <string>

template <typename... Args>
auto sum(Args... args) { return (args + ...); }              // ((a + b) + c) + ...

template <typename... Args>
void print_all(const Args&... args) {
    ((std::cout << args << ' '), ...);                       // fold over the comma operator
    std::cout << '\n';
}

template <typename T, typename... Rest>
bool all_equal(const T& first, const Rest&... rest) { return ((first == rest) && ...); }

int main() {
    std::cout << sum(1, 2, 3, 4) << ' ' << sum(1.5, 2.5) << '\n';
    print_all("id", 7, 3.14, std::string("ok"), 'x');
    std::cout << std::boolalpha << all_equal(3, 3, 3) << ' ' << all_equal(3, 3, 4) << '\n';
    std::cout << "sizeof...(Args) for 4 args: " << [](auto... a) { return sizeof...(a); }(1, 2, 3, 4) << '\n';
}
```

This is how `std::make_unique<T>(args...)` and `std::format` accept any arguments. To pass arguments on unchanged (keeping rvalues as rvalues), templates use **perfect forwarding**: `template <typename... A> auto make(A&&... a) { return T(std::forward<A>(a)...); }`.

## Type traits: asking questions about types

`<type_traits>` answers questions at compile time, and `if constexpr` (chapter 24) chooses code based on the answers:

```cpp run
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
void copy_items(T* dst, const T* src, std::size_t n) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(dst, src, n * sizeof(T));         // plain bytes: one fast bulk copy
        std::cout << "memcpy path\n";
    } else {
        for (std::size_t i = 0; i < n; ++i) dst[i] = src[i];   // run each element's copy assignment
        std::cout << "element-wise path\n";
    }
}

int main() {
    int a[3] = {1, 2, 3}, b[3];
    copy_items(b, a, 3);
    std::string s[2] = {"x", "y"}, t[2];
    copy_items(t, s, 2);
    std::cout << std::boolalpha << std::is_integral_v<long> << ' ' << std::is_same_v<int, std::int32_t>
              << ' ' << std::is_pointer_v<int*> << '\n';
}
```

The decision happens at compile time, so each instantiation contains only one of the two branches.

## CRTP: static polymorphism without virtual calls

The **Curiously Recurring Template Pattern** lets a base class call the derived class's functions, resolved at compile time. You get polymorphic-style reuse with no vtable and full inlining:

```cpp run
#include <iostream>

template <typename Derived>
class Shape {                                 // the base knows its derived type
public:
    double area() const { return static_cast<const Derived&>(*this).area_impl(); }
    void describe() const { std::cout << Derived::name << " with area " << area() << '\n'; }
};

class Circle : public Shape<Circle> {
public:
    static constexpr const char* name = "circle";
    explicit Circle(double r) : r_(r) {}
    double area_impl() const { return 3.14159265 * r_ * r_; }
private:
    double r_;
};

class Square : public Shape<Square> {
public:
    static constexpr const char* name = "square";
    explicit Square(double s) : s_(s) {}
    double area_impl() const { return s_ * s_; }
private:
    double s_;
};

template <typename S>
void report(const Shape<S>& s) { s.describe(); }   // works for any Shape, with no virtual call

int main() {
    report(Circle{1});
    report(Square{3});
}
```

The trade-off compared with virtual functions: no vtable and full inlining, but no single runtime container of mixed shapes. Engines use CRTP in maths types and component systems; Eigen uses it throughout.

## Keeping templates under control

- **Constrain** templates with concepts. Errors become readable, and overloads become clear.
- **Explicit instantiation** (`template class Matrix<float>;` in one `.cpp`, plus `extern template class Matrix<float>;` in the header) compiles a common instantiation once, cutting build times.
- Not everything should be a template. A function taking `std::span<const double>` is often simpler than a template over every container type.

::quiz
? What is the main benefit of `template <std::integral T>` over plain `template <typename T>`?
+ Wrong types are rejected at the call site with a clear message
- It runs faster
- It allows more types
> Concepts move the requirement into the signature.
? What does `(args + ...)` do for args 1, 2, 3?
+ Computes ((1 + 2) + 3)
- Concatenates them as text
- Creates a tuple
> It is a fold expression over +.
? CRTP replaces which run-time mechanism with a compile-time one?
+ Virtual function dispatch
- Exceptions
- Heap allocation
> The base casts itself to Derived, so calls are resolved and inlined at compile time.
::
