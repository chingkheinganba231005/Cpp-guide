C++ can run code **while compiling**. Results computed at compile time cost nothing at run time, and mistakes in them become compile errors. Game engines precompute lookup tables, trading systems bake in protocol layouts, and libraries validate format strings, all before the program starts.

## constexpr variables and functions

```cpp run
#include <array>
#include <iostream>

constexpr long long factorial(int n) {         // may run at compile time or at run time
    long long r = 1;
    for (int i = 2; i <= n; ++i) r *= i;
    return r;
}

int main() {
    constexpr long long f10 = factorial(10);   // computed by the compiler: the program contains the number 3628800
    static_assert(factorial(5) == 120);        // checked while compiling; a wrong value is a compile error
    std::array<int, factorial(4)> slots{};     // usable where a compile-time constant is required
    int n = 12;
    std::cout << f10 << ' ' << slots.size() << ' ' << factorial(n) << " (this one at run time)\n";
}
```

- A `constexpr` **variable** must be computed at compile time.
- A `constexpr` **function** *can* run at compile time when its arguments are compile-time constants, and also works normally at run time.
- `static_assert(condition)` checks a condition during compilation. It is a test that runs every time you build.

Modern `constexpr` functions can use loops, local variables, `if`, and even `std::vector` and `std::string` internally (C++20), as long as everything is finished by the end of the evaluation.

## consteval: compile time only

```cpp run expect=fail
consteval int checked_port(int p) {
    if (p < 1 || p > 65535) throw "port out of range";   // reaching a throw at compile time = compile error
    return p;
}

int main() {
    constexpr int ok = checked_port(8080);
    constexpr int bad = checked_port(70000);   // rejected while compiling
    return ok + bad;
}
```

`consteval` functions must run at compile time. They are ideal for validating configuration constants. `std::format` uses this technique to check format strings at compile time.

## Compile-time lookup tables

Precomputing a table at compile time is a classic engine and DSP trick:

```cpp run
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

constexpr std::array<std::uint32_t, 256> make_crc_table() {      // CRC-32, used in zip files and network protocols
    std::array<std::uint32_t, 256> t{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t c = i;
        for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        t[i] = c;
    }
    return t;
}
constexpr auto CRC_TABLE = make_crc_table();                     // built by the compiler, stored in the binary

constexpr std::uint32_t crc32(const char* s, std::size_t n) {
    std::uint32_t c = 0xFFFFFFFFu;
    for (std::size_t i = 0; i < n; ++i) c = CRC_TABLE[(c ^ static_cast<std::uint8_t>(s[i])) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

int main() {
    static_assert(crc32("123456789", 9) == 0xCBF43926u);         // the standard CRC-32 check value, verified at compile time
    std::cout << std::hex << crc32("hello", 5) << '\n';
}
```

## if constexpr: choose code at compile time

`if constexpr` discards the branch that isn't taken, so it may even contain code that wouldn't compile for this type:

```cpp run
#include <iostream>
#include <string>
#include <type_traits>

template <typename T>
std::string describe(const T& value) {
    if constexpr (std::is_arithmetic_v<T>) return "number " + std::to_string(value);
    else if constexpr (std::is_same_v<T, std::string>) return "text '" + value + "'";
    else return "something else";
}

struct Opaque {};

int main() {
    std::cout << describe(42) << ", " << describe(2.5) << ", " << describe(std::string("hi")) << ", " << describe(Opaque{}) << '\n';
}
```

## Where compile-time programming pays off

| Use | Example |
|---|---|
| Lookup tables | CRC, sine tables, gamma curves, bit-reversal for FFTs |
| Unit-safe maths | `std::chrono` durations, physical units libraries |
| Protocol layouts | message sizes and field offsets checked with `static_assert` |
| Configuration | `constexpr` constants validated with `consteval` |
| Generic code | `if constexpr` choosing the fastest path for each type |

:::warn
Compile-time code makes builds slower and errors harder to read. Use it where it buys correctness or measurable speed, not everywhere.
:::

::quiz
? What does `static_assert(sizeof(Order) == 32);` do?
+ Fails the build if Order is not exactly 32 bytes
- Checks at run time
- Sets the size of Order
> It is a compile-time test, used to protect memory layouts.
? Difference between constexpr and consteval functions?
+ constexpr may run at compile time or at run time; consteval must run at compile time
- They are identical
- consteval runs only at run time
> consteval is an "immediate function".
::
