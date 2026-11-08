Things go wrong: files are missing, input is malformed, connections drop. C++ gives you several ways to report failure, and choosing the right one is part of good design. This chapter covers **exceptions**, **`std::optional`**, **`std::expected`** and **assertions**.

## Exceptions

An exception stops the normal flow, unwinds the stack (running every destructor on the way, which is RAII at work), and jumps to the nearest matching `catch`:

```cpp run
#include <iostream>
#include <stdexcept>
#include <string>

double parse_price(const std::string& s) {
    std::size_t used = 0;
    double v = std::stod(s, &used);                 // throws std::invalid_argument if s isn't a number
    if (used != s.size()) throw std::invalid_argument("trailing characters in '" + s + "'");
    if (v <= 0) throw std::out_of_range("price must be positive");
    return v;
}

int main() {
    for (std::string s : {"101.25", "abc", "12x", "-5"}) {
        try {
            double price = parse_price(s);          // if this throws, the next line never runs
            std::cout << "parsed " << price << '\n';
        } catch (const std::invalid_argument& e) {
            std::cout << "invalid: " << e.what() << '\n';
        } catch (const std::exception& e) {         // the base of all standard exceptions
            std::cout << "error: " << e.what() << '\n';
        }
    }
}
```

:::python
This is almost exactly Python's `try`/`except`/`raise`. Differences: you can throw any type, but always throw classes derived from `std::exception`; always catch by **const reference**; and there is no `finally`, because destructors do the cleanup.
:::

Your own exception types usually derive from a standard one:

```cpp norun
struct ConfigError : std::runtime_error {
    using std::runtime_error::runtime_error;    // reuse the constructor that takes a message
};
throw ConfigError("missing key 'port'");
```

## When to use exceptions

Exceptions are for **exceptional** situations that the immediate caller usually can't handle: a corrupted file, out of memory, a broken invariant. When nothing is thrown they cost essentially nothing; actually throwing one costs microseconds.

That's why **low-latency systems** (trading engines, game frame loops, real-time audio) avoid throwing on their hot paths, and some projects compile with exceptions disabled (`-fno-exceptions`). For failures that are part of normal operation, such as “no order at that price” or “the key isn't in the cache”, return a value that *says* it failed.

## std::optional: maybe a value

```cpp run
#include <iostream>
#include <optional>
#include <string>
#include <vector>

struct User { int id; std::string name; };

std::optional<User> find_user(const std::vector<User>& users, int id) {
    for (const auto& u : users) if (u.id == id) return u;
    return std::nullopt;                             // "nothing", like returning None
}

int main() {
    std::vector<User> users{{1, "ada"}, {2, "linus"}};
    if (auto u = find_user(users, 2)) std::cout << "found " << u->name << '\n';
    auto missing = find_user(users, 9);
    std::cout << "user 9: " << (missing ? missing->name : "not found") << '\n';
    std::cout << find_user(users, 9).value_or(User{0, "guest"}).name << '\n';
}
```

`std::optional<T>` holds either a `T` or nothing, with no heap allocation. Use it when “not found” is a normal answer.

## std::expected: a value or an error (C++23)

When the caller needs to know *why* something failed, return `std::expected<T, E>`: either the value, or an error of type `E`:

```cpp run
#include <charconv>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>

enum class ParseError { Empty, NotANumber, OutOfRange };

std::string_view to_string(ParseError e) {
    switch (e) {
        case ParseError::Empty: return "empty input";
        case ParseError::NotANumber: return "not a number";
        case ParseError::OutOfRange: return "out of range";
    }
    return "?";
}

std::expected<int, ParseError> parse_qty(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    int value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);   // fast, no exceptions, no allocation
    if (ec == std::errc::result_out_of_range) return std::unexpected(ParseError::OutOfRange);
    if (ec != std::errc{} || ptr != s.data() + s.size()) return std::unexpected(ParseError::NotANumber);
    return value;
}

int main() {
    for (std::string_view s : {"250", "", "12a", "99999999999"}) {
        auto r = parse_qty(s);
        if (r) std::cout << "'" << s << "' -> " << *r << '\n';
        else   std::cout << "'" << s << "' -> error: " << to_string(r.error()) << '\n';
    }
}
```

`std::from_chars` is the fastest way to parse numbers in C++: no locale, no exceptions, no allocation. Trading systems parse market data with it (chapter 33).

## Assertions: catching programmer mistakes

An **assertion** checks something that must be true if the program is correct, such as an invariant or a precondition. If it is false, the program has a bug and stops immediately:

```cpp run expect=crash
#include <cassert>
#include <vector>

double average(const std::vector<double>& v) {
    assert(!v.empty() && "average of an empty vector");   // a precondition
    double s = 0; for (double x : v) s += x;
    return s / v.size();
}

int main() {
    std::vector<double> none;
    return static_cast<int>(average(none));
}
```

`assert` is removed entirely when compiling with `-DNDEBUG` (typical for release builds), so it costs nothing in production. Use it for bugs; use exceptions, `optional` or `expected` for bad input and external failures.

## Choosing

| Situation | Tool |
|---|---|
| A bug (broken invariant, impossible state) | `assert`, or a contract check that terminates |
| A normal “not found” or “no result” | `std::optional<T>` |
| A failure the caller should react to, with a reason | `std::expected<T, E>`, or an error code |
| A rare, serious failure far from where it can be handled | an exception |
| A hot path with strict latency (trading, audio, a game frame) | no exceptions; return codes, `expected`, or pre-validated input |

## Exception safety, in one paragraph

If an exception leaves a function halfway, are your objects still valid? With RAII members, the **basic guarantee** (no leaks, objects still usable) comes almost for free. For the **strong guarantee** (all or nothing), do the work on a copy and then commit with a non-throwing swap, the copy-and-swap idiom from chapter 15. Mark functions that cannot throw as `noexcept`; the library uses this to choose faster code paths, such as moving instead of copying when a vector grows.

::quiz
? A function looks up a username that may legitimately not exist. What should it return?
+ std::optional<User>
- throw an exception every time
- a null reference
> Absence is a normal outcome, so represent it in the return type.
? What does `assert` do in a release build compiled with -DNDEBUG?
+ Nothing: the check is removed
- It throws an exception
- It logs a warning
> Assertions are for catching bugs during development and testing.
? Why do low-latency systems avoid throwing on hot paths?
+ Throwing and unwinding take microseconds and are unpredictable
- C++ exceptions are unsafe
- try blocks are slow even when nothing is thrown
> On the non-throwing path exceptions cost almost nothing; the throw itself is expensive.
::
