Text in C++ lives in `std::string`. It behaves much like Python's `str`, except that it is **mutable** (you can change it in place) and each character is one `char`.

## std::string basics

```cpp run
#include <iostream>
#include <string>

int main() {
    std::string name = "Ada";
    std::string greeting = "Hello, " + name + "!";      // + concatenates
    std::cout << greeting << '\n';
    std::cout << "length: " << greeting.size() << '\n';  // Python: len(greeting)
    std::cout << "first: " << greeting[0] << ", last: " << greeting.back() << '\n';
    greeting[0] = 'J';                                    // strings are mutable
    greeting += " Welcome.";                              // append in place
    std::cout << greeting << '\n';
}
```

| Python | C++ `std::string` |
|---|---|
| `len(s)` | `s.size()` |
| `s[i]` | `s[i]` (no negative indices; `s.back()` for the last) |
| `s[1:4]` | `s.substr(1, 3)` (start, **length**) |
| `"lo" in s` | `s.find("lo") != std::string::npos`, or `s.contains("lo")` (C++23) |
| `s.startswith("He")` | `s.starts_with("He")` (C++20) |
| `s.upper()` | loop with `std::toupper` (below) |
| `str(42)`, `int("42")` | `std::to_string(42)`, `std::stoi("42")` |
| `f"{x:.2f}"` | `std::format("{:.2f}", x)` (C++20) |

```cpp run
#include <cctype>
#include <iostream>
#include <string>

int main() {
    std::string s = "The quick brown fox";
    std::cout << s.substr(4, 5) << '\n';                       // "quick"
    std::size_t pos = s.find("brown");
    std::cout << "brown at index " << pos << '\n';
    std::cout << std::boolalpha << s.starts_with("The") << ' ' << s.contains("dog") << '\n';
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));   // in-place upper case
    std::cout << s << '\n';
    int n = std::stoi("123") + 1;
    std::cout << n << ' ' << std::to_string(3.5) << '\n';
}
```

:::warn Text in quotes is not a std::string
`"hello"` by itself is an older C-style string literal, a raw array of `char`. That is why `"Hello, " + "world"` does **not** compile: you can't add two arrays. Make one side a `std::string` (`std::string{"Hello, "} + "world"`), or use the `s` suffix: `using namespace std::literals; auto t = "Hello, "s + "world";`.
:::

## Formatting output with std::format

`std::format` (C++20) works like Python's f-strings and `str.format`:

```cpp run
#include <format>
#include <iostream>

int main() {
    double price = 3.14159;
    int qty = 7;
    std::cout << std::format("{} items at ${:.2f} = ${:.2f}\n", qty, price, qty * price);
    std::cout << std::format("|{:<8}|{:^8}|{:>8}|\n", "left", "mid", "right");
    std::cout << std::format("{:08.3f} {:x} {:b} {:e}\n", price, 255, 5, 123456.0);
}
```

The format codes are almost identical to Python's: `:<8` left-align in 8 columns, `:.2f` two decimals, `:x` hexadecimal. C++23 adds `std::print` and `std::println`, which print directly. They are not yet in GCC 13's library, so this course uses `std::cout << std::format(...)`, which works everywhere.

## Reading input

`std::cin >>` reads one whitespace-separated value at a time, converting it to the variable's type:

```cpp run stdin="Ada 36"
#include <iostream>
#include <string>

int main() {
    std::string name;
    int age;
    std::cout << "Name and age? ";
    std::cin >> name >> age;
    std::cout << '\n' << name << " will be " << age + 1 << " next year\n";
}
```

To read a whole line, spaces included, use `std::getline`. To check whether reading worked (the user typed letters instead of a number?), test the stream: `if (std::cin >> age) { ... } else { ... }`.

```cpp run stdin="Ada Lovelace\n3 4 5\n"
#include <iostream>
#include <string>

int main() {
    std::string full;
    std::getline(std::cin, full);                 // reads "Ada Lovelace"
    int x, sum = 0;
    while (std::cin >> x) sum += x;               // read numbers until input ends
    std::cout << full << ": sum = " << sum << '\n';
}
```

## std::string_view: looking at text without copying

`std::string_view` is a lightweight **view** of characters owned by someone else: just a pointer and a length. Use it for function parameters that only *read* text. It accepts `std::string`s and literals without making a copy.

```cpp run
#include <iostream>
#include <string>
#include <string_view>

int count_vowels(std::string_view s) {
    int n = 0;
    for (char c : s) n += (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
    return n;
}

int main() {
    std::string owned = "programming";
    std::cout << count_vowels(owned) << ' ' << count_vowels("literal works too") << '\n';
}
```

:::warn
A `string_view` does not own its characters. Never return a `string_view` into a local `std::string`, and never keep one after the string it looks at has changed or been destroyed. Chapter 13 shows the crash this causes.
:::

## Characters are small numbers

```cpp run
#include <iostream>

int main() {
    char c = 'A';
    std::cout << c << " has code " << static_cast<int>(c) << '\n';
    std::cout << static_cast<char>(c + 2) << '\n';     // 'C'
    std::cout << ('7' - '0') << '\n';                  // turn a digit character into its value: 7
}
```

`std::string` holds bytes. Non-English text is usually stored as UTF-8, where one visible character may take several bytes, so `size()` counts bytes, not characters. Proper Unicode handling needs a library such as ICU.

:::exercise Word counter
Read a line of text and print how many words it has and the longest word. Hint: put the line into a `std::istringstream` (from `<sstream>`) and read words with `>>`.
---solution---
```cpp run stdin="the quick brown fox jumps over the lazy dog"
#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string line, word, longest;
    std::getline(std::cin, line);
    std::istringstream in(line);
    int count = 0;
    while (in >> word) {
        ++count;
        if (word.size() > longest.size()) longest = word;
    }
    std::cout << count << " words, longest: " << longest << '\n';
}
```
:::

::quiz
? What does `std::string("hello").substr(1, 3)` return?
+ "ell"
- "el"
- "ello"
> The second argument is a length, not an end index.
? Why prefer `std::string_view` for read-only text parameters?
+ It avoids copying the text
- It can change the text
- It owns the text
> A view is just a pointer and a length to text owned by someone else.
::
