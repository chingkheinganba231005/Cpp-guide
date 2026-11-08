You have been using templates since chapter 7: `std::vector<int>` and `std::vector<std::string>` are the same template, `std::vector<T>`, filled in with different types. Templates let you write code **once** and have the compiler generate a specialised, full-speed version for each type you use. They are the foundation of the standard library, and of every high-performance C++ library.

:::eli5
A template is a stencil. You write the stencil once with a blank where the type goes (`T`). When you use it with `int`, the compiler traces the stencil with `int` written in the blank and gets ordinary code for ints. With `double`, it traces another copy for doubles.
:::

## Function templates

```cpp run
#include <iostream>
#include <string>

template <typename T>
T largest(const T& a, const T& b) {
    return (a < b) ? b : a;
}

int main() {
    std::cout << largest(3, 7) << '\n';                                   // T = int
    std::cout << largest(2.5, 1.5) << '\n';                               // T = double
    std::cout << largest(std::string("pear"), std::string("apple")) << '\n';   // T = std::string
    std::cout << largest<double>(3, 2.5) << '\n';                         // T given explicitly
}
```

The compiler **deduces** `T` from the arguments. Each different `T` produces a separate compiled function, called an *instantiation*, exactly as fast as if you had written it by hand for that type.

:::python
Python functions are already “generic”: `def largest(a, b): return b if a < b else a` works for anything with `<`. But Python checks at run time, on every call. C++ templates do the checking and the specialisation **at compile time**, so there is zero run-time cost, and a type without `<` is a compile error rather than a crash. Chapter 23's *concepts* make those errors readable.
:::

## Class templates

```cpp run
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

template <typename T, std::size_t Capacity>      // a type parameter AND a value parameter
class FixedStack {
public:
    void push(const T& value) {
        if (size_ == Capacity) throw std::overflow_error("stack full");
        items_[size_++] = value;
    }
    T pop() {
        if (size_ == 0) throw std::underflow_error("stack empty");
        return items_[--size_];
    }
    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }
private:
    T items_[Capacity]{};
    std::size_t size_ = 0;
};

int main() {
    FixedStack<int, 4> s;                 // no heap at all: the storage is inside the object
    for (int i = 1; i <= 4; ++i) s.push(i * i);
    while (!s.empty()) std::cout << s.pop() << ' ';
    std::cout << '\n';
    FixedStack<std::string, 2> names;
    names.push("ada"); names.push("bjarne");
    std::cout << names.pop() << ", sizeof(FixedStack<int,4>) = " << sizeof(FixedStack<int, 4>) << '\n';
}
```

`std::size_t Capacity` is a **non-type template parameter**: a compile-time constant. `std::array<T, N>` works this way, and so do the fixed-size matrix types in game maths libraries (`Mat<float, 4, 4>`).

## Templates live in headers

The compiler needs the full template *definition* wherever it creates an instantiation. So templates are normally written entirely in header files (chapter 21), unlike ordinary functions, whose bodies go in `.cpp` files.

## Class template argument deduction (CTAD)

Since C++17 the compiler can deduce class template arguments from the constructor:

```cpp run
#include <iostream>
#include <utility>
#include <vector>

int main() {
    std::vector v{1, 2, 3};          // std::vector<int>
    std::pair p{2.5, 'x'};           // std::pair<double, char>
    std::cout << v.size() << ' ' << p.first << p.second << '\n';
}
```

## Specialisation: a custom version for one type

```cpp run
#include <iostream>
#include <string>

template <typename T>
std::string describe(const T&) { return "something"; }

template <>
std::string describe<bool>(const bool& b) { return b ? "yes" : "no"; }   // full specialisation for bool

int main() {
    std::cout << describe(42) << ' ' << describe(true) << '\n';
}
```

Specialisation is how libraries pick faster algorithms for particular types. For example, a copy routine can use one fast bulk memory copy for plain trivially-copyable types.

## Generic lambdas and auto parameters

A lambda with `auto` parameters is a template in disguise, and so is a normal function with `auto` parameters (C++20):

```cpp run
#include <iostream>
#include <string>

auto twice(auto x) { return x + x; }         // a function template, in short form

int main() {
    auto add = [](auto a, auto b) { return a + b; };
    std::cout << add(1, 2) << ' ' << add(1.5, 2.25) << ' ' << add(std::string("ab"), std::string("cd")) << '\n';
    std::cout << twice(21) << ' ' << twice(std::string("ha")) << '\n';
}
```

## Error messages, and the costs of templates

If you call a template with a type that doesn't support what the template does, the error appears **inside** the template, which can produce long, confusing messages:

```cpp run expect=fail
#include <iostream>

template <typename T>
T largest(const T& a, const T& b) { return (a < b) ? b : a; }

struct Point { int x, y; };          // no operator<

int main() {
    largest(Point{1, 2}, Point{3, 4});
}
```

Read it from the line that mentions **your** code (`required from here`) and the first `error:`: `no match for 'operator<'`. C++20 **concepts** (chapter 23) put the requirement on the template's signature, so the error says “Point does not satisfy totally_ordered” instead.

Templates also cost **compile time** and **binary size**, since every instantiation is a separate copy of the code. In large projects this matters, and chapter 23 discusses how to keep it under control.

:::exercise A generic statistics function
Write `template <typename Container> auto mean(const Container& c)` that works for `std::vector<int>`, `std::vector<double>` and `std::array<float, 3>`, returning a `double`. Use a range-for loop.
---solution---
```cpp run
#include <array>
#include <iostream>
#include <vector>

template <typename Container>
double mean(const Container& c) {
    double sum = 0;
    std::size_t n = 0;
    for (const auto& x : c) { sum += x; ++n; }
    return n ? sum / n : 0.0;
}

int main() {
    std::vector<int> a{1, 2, 3, 4};
    std::vector<double> b{0.5, 1.5};
    std::array<float, 3> c{1.f, 1.f, 4.f};
    std::cout << mean(a) << ' ' << mean(b) << ' ' << mean(c) << '\n';
}
```
:::

::quiz
? When is a template turned into real machine code?
+ At compile time, once for each set of template arguments used
- At run time, like Python
- Never; templates are interpreted
> Each instantiation is ordinary, fully optimised code.
? Why are templates usually defined entirely in header files?
+ The compiler needs the full definition at each place it instantiates the template
- Headers compile faster
- It's a style rule only
> A .cpp file elsewhere can't instantiate a template whose body it can't see.
::
