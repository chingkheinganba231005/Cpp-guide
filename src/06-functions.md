A function packages a piece of work under a name, so you can reuse it and reason about it separately. C++ functions look like Python's `def`, with types added.

## Defining and calling

```cpp run
#include <iostream>
#include <string>

double area_of_circle(double radius) {     // return type, name, (parameter type and name)
    const double pi = 3.141592653589793;
    return pi * radius * radius;
}

void greet(const std::string& name) {      // void: returns nothing (chapter 9 explains const&)
    std::cout << "Hello, " << name << "!\n";
}

int main() {
    std::cout << area_of_circle(2.0) << '\n';
    greet("Ada");
}
```

Every parameter has a type, and so does the result. `void` means “no result”.

:::python
```python
def area_of_circle(radius: float) -> float:
    return 3.14159 * radius * radius
```
Python type hints are optional suggestions. C++ types are checked: calling `area_of_circle("two")` is a compile error, not a runtime surprise.
:::

## Declare before use

The compiler reads top to bottom. A function must be at least **declared** before it is called. A declaration (a *prototype*) is the first line with a semicolon:

```cpp run
#include <iostream>

int square(int x);          // declaration: "this exists, defined later"

int main() {
    std::cout << square(9) << '\n';
}

int square(int x) {         // definition
    return x * x;
}
```

This is the idea behind **header files** (chapter 21): declarations go in `.hpp` files that others `#include`, and definitions go in `.cpp` files.

## Pass by value: functions get copies

By default, a parameter is a **copy** of the argument. Changing it inside the function does not affect the caller:

```cpp run predict
#include <iostream>

void try_to_change(int n) {
    n = 100;
}

int main() {
    int x = 1;
    try_to_change(x);
    std::cout << x << '\n';
}
```

To let a function modify the caller's variable, pass by **reference** (`int& n`). To avoid copying large objects such as strings and vectors, pass by **const reference** (`const std::string& s`). Chapter 9 covers both in depth. Rule of thumb for now: small built-in types (`int`, `double`) by value, big things by `const&`.

## Overloading: same name, different parameter types

```cpp run
#include <iostream>
#include <string>

int twice(int x) { return 2 * x; }
double twice(double x) { return 2 * x; }
std::string twice(const std::string& s) { return s + s; }

int main() {
    std::cout << twice(21) << ' ' << twice(1.25) << ' ' << twice(std::string{"ab"}) << '\n';
}
```

The compiler picks the version whose parameter types match the arguments. Python can't do this; you would check types by hand.

## Default arguments

```cpp run
#include <iostream>
#include <string>

std::string repeat(const std::string& s, int times = 2, const std::string& sep = " ") {
    std::string out;
    for (int i = 0; i < times; ++i) out += (i ? sep : "") + s;
    return out;
}

int main() {
    std::cout << repeat("ha") << " | " << repeat("ha", 4) << " | " << repeat("ha", 3, "-") << '\n';
}
```

Defaults must come last. C++ has no keyword arguments, so you cannot skip `times` and only set `sep`. Chapter 14 shows how a small struct of options solves that.

## Returning several values

Return a struct, or `std::pair`/`std::tuple`, and unpack with **structured bindings** (like Python's tuple unpacking):

```cpp run
#include <iostream>
#include <utility>

std::pair<int, int> divide(int a, int b) {
    return {a / b, a % b};
}

int main() {
    auto [quotient, remainder] = divide(17, 5);    // Python: q, r = divmod(17, 5)
    std::cout << quotient << " r " << remainder << '\n';
}
```

## Recursion

A function may call itself. Each call gets its own copy of its local variables in its own **stack frame** (chapter 8 shows this visually).

```cpp run
#include <cstdint>
#include <iostream>

std::uint64_t factorial(int n) {
    if (n <= 1) return 1;            // base case: stops the recursion
    return n * factorial(n - 1);     // recursive case
}

int main() {
    for (int n : {5, 10, 20}) std::cout << n << "! = " << factorial(n) << '\n';
}
```

`20!` fits in 64 bits but `21!` does not: remember the integer limits from chapter 2.

## Small functions are free

Calling a function in Python has a noticeable cost. In C++ the optimiser usually **inlines** small functions, pasting their body into the caller. So splitting code into small, well-named functions costs nothing at runtime. Write them freely.

## Lambdas: functions without a name

A **lambda** is a function written inline, like Python's `lambda` but able to hold several statements:

```cpp run
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{5, -3, 8, -1, 2};
    auto by_abs = [](int a, int b) { return std::abs(a) < std::abs(b); };
    std::sort(v.begin(), v.end(), by_abs);         // Python: sorted(v, key=abs)
    for (int x : v) std::cout << x << ' ';
    std::cout << '\n';

    int threshold = 2;
    auto count = std::count_if(v.begin(), v.end(), [threshold](int x) { return x > threshold; });
    std::cout << count << " values above " << threshold << '\n';
}
```

The `[ ]` part is the **capture list**: which outside variables the lambda may use (`threshold` here). Chapter 19 explains captures fully.

:::exercise Greatest common divisor
Write `int gcd(int a, int b)` using Euclid's rule: gcd(a, 0) = a, otherwise gcd(a, b) = gcd(b, a % b). Print gcd(1071, 462).
---solution---
```cpp run
#include <iostream>

int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }

int main() { std::cout << gcd(1071, 462) << '\n'; }
```
:::

::quiz
? `void f(int n) { n = 5; }` is called as `f(x)`. What happens to x?
+ Nothing: f changed its own copy
- x becomes 5
- Compile error
> Parameters are copies unless declared as references.
? Why do C++ programs often put declarations above main or in headers?
+ The compiler must see a declaration before a call
- It makes the program faster
- main must be last
> Compilation reads top to bottom, so a function must be declared before it is used.
::
