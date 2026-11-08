C++11 through C++23 added many features that make code shorter, safer and faster. You have met some already (`auto`, lambdas, smart pointers, range-for). This chapter collects the everyday modern toolkit in one place: the things expert code uses constantly.

## Structured bindings

Unpack pairs, tuples, structs and map entries into named variables:

```cpp run
#include <iostream>
#include <map>
#include <string>
#include <tuple>

struct Quote { double bid, ask; };

std::tuple<int, std::string, double> load() { return {7, "AAPL", 189.5}; }

int main() {
    auto [id, symbol, price] = load();                       // like Python: id, symbol, price = load()
    auto [bid, ask] = Quote{99.5, 100.5};
    std::cout << id << ' ' << symbol << ' ' << price << ", spread " << ask - bid << '\n';
    std::map<std::string, int> stock{{"apples", 3}, {"pears", 5}};
    for (const auto& [fruit, n] : stock) std::cout << fruit << ": " << n << '\n';
}
```

## enum class: safe enumerations

```cpp run
#include <cstdint>
#include <iostream>

enum class Side : std::uint8_t { Buy, Sell };          // scoped, typed, 1 byte
enum class Color { Red, Green, Blue };

const char* to_string(Side s) { return s == Side::Buy ? "BUY" : "SELL"; }

int main() {
    Side s = Side::Sell;
    std::cout << to_string(s) << ", size " << sizeof(Side) << " byte\n";
    // if (s == Color::Red) {}   // compile error: different enum types can't be mixed
    // int n = s;                // compile error: no silent conversion to int
    std::cout << static_cast<int>(Color::Blue) << '\n';      // explicit when you really need the number
}
```

Old-style `enum` (without `class`) leaks its names into the surrounding scope and converts silently to `int`. Always use `enum class`.

## if and switch with initialisers

```cpp run
#include <iostream>
#include <map>
#include <string>

int main() {
    std::map<std::string, double> prices{{"BTC", 64000.0}, {"ETH", 3100.0}};
    if (auto it = prices.find("ETH"); it != prices.end())   // `it` exists only inside this if/else
        std::cout << "ETH " << it->second << '\n';
    else
        std::cout << "not found\n";
}
```

## std::variant: one of several types

A type-safe union: the value is exactly one of the listed alternatives, stored inline with no heap allocation. Combined with `std::visit` and the “overloaded” trick, it gives you pattern matching:

```cpp run
#include <iostream>
#include <string>
#include <variant>
#include <vector>

struct NewOrder { int id; double price; int qty; };
struct Cancel   { int id; };
struct Modify   { int id; int new_qty; };
using Message = std::variant<NewOrder, Cancel, Modify>;   // e.g. messages arriving from an exchange

template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };   // combine lambdas into one visitor

int main() {
    std::vector<Message> feed{NewOrder{1, 101.5, 10}, Modify{1, 7}, Cancel{1}};
    for (const auto& m : feed) {
        std::visit(overloaded{
            [](const NewOrder& o) { std::cout << "new #" << o.id << " " << o.qty << " @ " << o.price << '\n'; },
            [](const Cancel& c)   { std::cout << "cancel #" << c.id << '\n'; },
            [](const Modify& m)   { std::cout << "modify #" << m.id << " -> qty " << m.new_qty << '\n'; },
        }, m);
    }
    std::cout << "sizeof(Message) = " << sizeof(Message) << " bytes (largest alternative + a tag)\n";
}
```

If you forget to handle an alternative, the code doesn't compile. Variants are ideal for messages, AST nodes, events and states: any closed set of possibilities.

## std::span: a view of contiguous data

`std::span<T>` is a pointer plus a length: a non-owning view of a vector, array, C array or part of one. It is the modern way to pass “some contiguous elements” to a function, with no copy and no templates:

```cpp run
#include <array>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

double mean(std::span<const double> xs) {
    return xs.empty() ? 0.0 : std::accumulate(xs.begin(), xs.end(), 0.0) / xs.size();
}

int main() {
    std::vector<double> v{1, 2, 3, 4, 5, 6};
    std::array<double, 3> a{10, 20, 30};
    double c[] = {7, 8};
    std::cout << mean(v) << ' ' << mean(a) << ' ' << mean(c) << '\n';
    std::cout << mean(std::span(v).subspan(2, 3)) << " (elements 2..4 only, no copy)\n";
}
```

`span` and `string_view` are the two “view” types you pass by value: they are cheap to copy (16 bytes) and they don't own anything.

## Ranges and views: lazy pipelines

C++20 **views** compose operations lazily, like Python generators, with no temporary containers:

```cpp run
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto pipeline = v | std::views::filter([](int x) { return x % 2 == 0; })
                      | std::views::transform([](int x) { return x * x; })
                      | std::views::take(3);
    for (int x : pipeline) std::cout << x << ' ';                 // computed only as you iterate
    std::cout << '\n';
    for (int i : std::views::iota(0, 5) | std::views::reverse) std::cout << i << ' ';   // like reversed(range(5))
    std::cout << '\n';
    for (auto [i, x] : std::views::enumerate(std::vector<char>{'a', 'b', 'c'})) std::cout << i << x << ' ';   // C++23
    std::cout << '\n';
}
```

## Other everyday modern features

```cpp run
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

[[nodiscard]] int compute() { return 42; }        // warn if the caller ignores the result

struct Config {                                    // designated initialisers (C++20): name the fields
    int width = 800;
    int height = 600;
    bool vsync = true;
};

int main() {
    Config c{.width = 1920, .height = 1080};
    std::cout << c.width << 'x' << c.height << ' ' << std::boolalpha << c.vsync << '\n';
    auto n = compute();
    std::vector<std::string> names{"b", "a"};
    std::ranges::sort(names);
    std::cout << n << ' ' << names[0] << '\n';
    using namespace std::literals;
    auto s = "a std::string literal"s;              // the s suffix makes a std::string
    std::cout << s.size() << '\n';
}
```

| Feature | Why it matters |
|---|---|
| `[[nodiscard]]` | ignoring an error code or result becomes a warning |
| designated initialisers | readable configuration structs; this is how C++ does keyword arguments |
| `std::format` | safe, Python-like formatting (chapter 6) |
| `std::chrono` literals (`250ms`, `2s`) | units that can't be mixed up |
| `<numbers>` (`std::numbers::pi`) | exact maths constants |
| `std::bit_cast`, `<bit>` (`std::popcount`) | safe bit-level reinterpretation and fast bit operations |

::quiz
? Why prefer `enum class` over a plain enum?
+ Its names are scoped and it does not convert silently to int
- It is faster
- It can hold strings
> Scoping and type safety prevent mixing unrelated enums.
? What does std::span<const double> own?
+ Nothing: it views someone else's contiguous data
- A copy of the data
- A heap buffer
> Like string_view, it is a pointer and a length.
? A function receives a std::variant<NewOrder, Cancel>. How do you act on whichever alternative it holds?
+ std::visit with a visitor that handles every alternative
- A dynamic_cast
- Compare its address
> visit checks at compile time that every alternative is handled.
::
