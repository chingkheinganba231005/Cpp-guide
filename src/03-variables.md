A **variable** is a named piece of memory that holds a value. In C++ every variable has a **type**, and the type decides three things: how many bytes it uses, which values it can hold, and which operations make sense on it.

## Declaring variables

```cpp run
#include <iostream>

int main() {
    int age = 21;             // a whole number
    double height = 1.75;     // a number with a fractional part
    char grade = 'A';         // a single character
    bool likes_cpp = true;    // true or false

    std::cout << age << ' ' << height << ' ' << grade << ' ' << likes_cpp << '\n';
    age = age + 1;            // change it later: the type stays int
    std::cout << "next year: " << age << '\n';
}
```

The pattern is always **type name = initial value;**. Notice `true` prints as `1`. To print `true`/`false`, use `std::cout << std::boolalpha`.

:::python
Python: `age = 21` creates an object and a name pointing at it. C++: `int age = 21;` reserves a 4-byte box called `age`, right here, and puts 21 in it. You can never put a string in that box later.
:::

## The fundamental types

| Type | Typical size | Holds | Example |
|---|---|---|---|
| `bool` | 1 byte | `true` or `false` | `bool done = false;` |
| `char` | 1 byte | one character (really a small integer) | `char c = 'x';` |
| `int` | 4 bytes | whole numbers, about ±2.1 billion | `int n = -42;` |
| `long long` | 8 bytes | whole numbers, about ±9.2 × 10¹⁸ | `long long big = 9'000'000'000;` |
| `unsigned int` | 4 bytes | 0 to about 4.29 billion (no negatives) | `unsigned u = 7;` |
| `float` | 4 bytes | ~7 significant digits | `float f = 3.14f;` |
| `double` | 8 bytes | ~16 significant digits (the default for decimals) | `double d = 3.14;` |

Sizes are “typical” because the C++ standard only guarantees minimums. When the exact size matters (file formats, networking, GPUs) use the **fixed-width types** from `<cstdint>`: `std::int8_t`, `std::int32_t`, `std::uint64_t` and so on. `sizeof` tells you the size in bytes:

```cpp run
#include <cstdint>
#include <iostream>

int main() {
    std::cout << "bool " << sizeof(bool) << ", char " << sizeof(char) << ", int " << sizeof(int)
              << ", long long " << sizeof(long long) << ", float " << sizeof(float)
              << ", double " << sizeof(double) << ", int64_t " << sizeof(std::int64_t) << '\n';
}
```

## How numbers are really stored

Everything in memory is **bits** (0s and 1s), grouped into **bytes** of 8 bits. An `int` is 32 bits. Negative integers use a scheme called *two's complement*, and decimals use the IEEE 754 *floating-point* format. Play with it: try `127`, `128`, `-1`, `0.1`, `16777217`.

<div data-widget="bits" data-value="13" data-title="Type a number and see its bits"></div>

Two consequences every C++ programmer must know:

**1. Integers have limits, and going past them is a bug.** Python integers grow forever; C++ integers do not.

```cpp run
#include <iostream>
#include <limits>

int main() {
    std::cout << "largest int:  " << std::numeric_limits<int>::max() << '\n';
    unsigned int u = 0;
    u = u - 1;                       // unsigned arithmetic wraps around (well defined)
    std::cout << "0u - 1 = " << u << '\n';
    long long safe = 2'000'000'000LL * 2;   // use a wider type when results may be big
    std::cout << "2e9 * 2 as long long = " << safe << '\n';
}
```

For *signed* integers (`int`, `long long`), overflowing is **undefined behaviour**: the program may do anything. Chapter 13 explains what that means and how tools catch it. The rule for now: if a value might exceed two billion, use `long long` or `std::int64_t`.

**2. Floating-point numbers are approximations.** `0.1` cannot be stored exactly in binary, just as 1/3 cannot be written exactly in decimal.

```cpp run predict
#include <iostream>
#include <iomanip>

int main() {
    double a = 0.1 + 0.2;
    std::cout << std::setprecision(17) << a << '\n';
    std::cout << std::boolalpha << (a == 0.3) << '\n';
}
```

Never compare floating-point results with `==`. Check that they are *close*: `std::abs(a - b) < 1e-9`. (Python has exactly the same behaviour, because it uses the same IEEE doubles.)

## Initialising: always give a value

```cpp norun
int a = 5;      // copy-initialisation: the classic form
int b{5};       // brace-initialisation: the modern form
int c{};        // value-initialised: 0
int d;          // NOT initialised: contains garbage. Avoid!
```

Brace-initialisation has a safety bonus: it refuses *narrowing* conversions that lose information.

```cpp run expect=fail flags="-pedantic-errors"
int main() {
    double price = 9.99;
    int a = price;    // compiles: silently drops the .99
    int b{price};     // error: narrowing conversion
}
```

:::warn
Reading a variable you never initialised (`int d; std::cout << d;`) is undefined behaviour. Python would raise a `NameError`. C++ just reads whatever bits were left in that memory. Always initialise.
:::

The C++ standard says narrowing inside braces is an error. Clang and MSVC treat it as one. GCC only warns unless you add `-pedantic-errors`, which this example does. Adding `-pedantic-errors` to your own builds is a good habit.

## `auto`: let the compiler work out the type

```cpp run
#include <iostream>
#include <string>

int main() {
    auto n = 42;          // int
    auto x = 2.5;         // double
    auto big = 42LL;      // long long (the LL suffix)
    auto s = std::string{"hi"};
    std::cout << n << ' ' << x << ' ' << big << ' ' << s << '\n';
}
```

`auto` does **not** make a variable dynamic like Python: the type is still fixed, and the compiler just fills it in from the initial value. Use it when the type is obvious or very long. Write the type explicitly when it documents intent.

## `const` and `constexpr`: values that never change

```cpp run expect=fail
int main() {
    const double pi = 3.14159;       // cannot be changed after initialisation
    constexpr int days_per_week = 7; // known at compile time
    pi = 3.0;                        // error: assignment of read-only variable
}
```

Make everything `const` unless it needs to change. It prevents accidents and tells readers “this is a fixed fact”. `constexpr` goes further: the value is computed by the compiler (chapter 24).

## Converting between types

```cpp run
#include <iostream>

int main() {
    double d = 7.9;
    int i = static_cast<int>(d);      // explicit conversion: truncates towards zero
    std::cout << i << '\n';
    int total = 7, count = 2;
    std::cout << total / count << " vs " << static_cast<double>(total) / count << '\n';
}
```

`static_cast<type>(value)` is the safe, searchable way to convert. You will also see the C-style `(int)d`; avoid it in new code because it can do dangerous conversions silently.

:::exercise Temperature table
Print a table of Celsius values 0, 10, …, 100 and their Fahrenheit equivalents (F = C × 9/5 + 32). Use `double` for the result. You will need a loop, which is next chapter's topic, so for now print three lines by hand. Then try writing `9/5` instead of `9.0/5` and see what changes.
---solution---
```cpp run
#include <iostream>

int main() {
    double c = 25;
    std::cout << c << " C = " << c * 9.0 / 5 + 32 << " F\n";
    std::cout << c << " C with integer 9/5: " << c * (9 / 5) + 32 << " F (wrong: 9/5 is 1)\n";
}
```
:::

::quiz
? What does `int x = 7 / 2;` store?
- 3.5
+ 3
- 4
> Integer division discards the fractional part. 7.0 / 2 would give 3.5.
? Why should money-sensitive or equality checks avoid `double == double`?
+ Because most decimal fractions cannot be stored exactly in binary
- Because doubles are only 4 bytes
- Because == does not work on doubles
> 0.1 + 0.2 is 0.30000000000000004. Compare with a tolerance, or use integers (e.g. cents).
? Which declaration is a bug waiting to happen?
- `int a{};`
- `auto b = 1.0;`
+ `int c;` (then printing c)
> c is uninitialised. Reading it is undefined behaviour.
::
