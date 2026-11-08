Operators combine values into new values: `+`, `<`, `&&` and friends. Most work like Python's, with a few differences that cause real bugs.

## Arithmetic

```cpp run
#include <iostream>

int main() {
    int a = 17, b = 5;
    std::cout << a + b << ' ' << a - b << ' ' << a * b << '\n';
    std::cout << a / b << '\n';     // integer division: 3 (the .4 is thrown away)
    std::cout << a % b << '\n';     // remainder: 2
    std::cout << 17.0 / 5 << '\n';  // 3.4: one double operand makes it floating-point
    std::cout << -7 / 2 << ' ' << -7 % 2 << '\n';  // C++ truncates towards zero
}
```

:::python
| Python | C++ | Note |
|---|---|---|
| `7 / 2` → `3.5` | `7 / 2` → `3` | In C++ `/` on two integers is integer division. |
| `7 // 2` → `3` | `7 / 2` | There is no `//` (it starts a comment!). |
| `-7 // 2` → `-4` | `-7 / 2` → `-3` | Python rounds down, C++ rounds towards zero. |
| `2 ** 10` | `std::pow(2, 10)` | There is no power operator. `pow` returns a double. |
| `x += 1` | `x += 1` or `++x` | C++ also has increment and decrement operators. |
:::

## Increment and compound assignment

```cpp run predict
#include <iostream>

int main() {
    int n = 5;
    n += 3;       // n = n + 3  -> 8
    n *= 2;       // n = n * 2  -> 16
    ++n;          // add one     -> 17
    int a = n++;  // a gets the OLD value (17), then n becomes 18
    int b = ++n;  // n becomes 19 first, then b gets 19
    std::cout << n << ' ' << a << ' ' << b << '\n';
}
```

`++n` (pre-increment) and `n++` (post-increment) both add one. They differ only in which value the *expression* produces. Using them in the middle of complicated expressions is a classic way to write confusing code. Use them as standalone statements.

## Comparison and logic

```cpp run
#include <iostream>

int main() {
    int age = 20;
    bool adult = age >= 18;
    bool teen = age >= 13 && age <= 19;     // && is "and"
    bool weekend = false, holiday = true;
    bool free_day = weekend || holiday;     // || is "or"
    std::cout << std::boolalpha << adult << ' ' << teen << ' ' << free_day << ' ' << !adult << '\n';
}
```

| Python | C++ |
|---|---|
| `and`, `or`, `not` | `&&`, `\|\|`, `!` (the words `and`, `or`, `not` also work in standard C++, but are rarely used) |
| `a == b`, `a != b`, `<`, `<=` | the same |
| `0 < x < 10` | **`0 < x && x < 10`**. In C++, `0 < x < 10` compiles but means something else! |

Like Python, `&&` and `||` **short-circuit**: in `p != nullptr && p->ok()`, the right side is only evaluated if the left is true. You will rely on this constantly.

:::warn Chained comparisons
`0 < x < 10` is evaluated as `(0 < x) < 10`. `(0 < x)` is `true` or `false`, which converts to 1 or 0, which is always less than 10. So the expression is always true. GCC warns about this with `-Wall`: one more reason to always turn warnings on.
:::

```cpp run
#include <iostream>

int main() {
    int x = 50;
    std::cout << std::boolalpha << (0 < x < 10) << '\n';        // true! (bug)
    std::cout << (0 < x && x < 10) << '\n';                     // false (correct)
}
```

## The assignment-in-condition trap

`=` assigns and `==` compares. Writing `if (x = 5)` assigns 5 to `x` and then tests 5, which is “true”. Compilers warn (`suggest parentheses around assignment used as truth value`). Read your warnings.

## Bitwise operators

C++ can work on the individual bits of integers, which matters for flags, hashing, graphics, networking and low-level performance work:

```cpp run
#include <bitset>
#include <iostream>

int main() {
    unsigned a = 0b1100, b = 0b1010;
    std::cout << std::bitset<4>(a & b) << " AND\n"
              << std::bitset<4>(a | b) << " OR\n"
              << std::bitset<4>(a ^ b) << " XOR\n"
              << std::bitset<8>(a << 2) << " shift left by 2 (times 4)\n"
              << std::bitset<4>(a >> 1) << " shift right by 1 (divide by 2)\n";
}
```

## Precedence: when in doubt, add parentheses

`*` and `/` bind tighter than `+` and `-`, and comparisons bind tighter than `&&`, which binds tighter than `||`, the same as in Python. The bitwise operators have surprising precedence: `x & 1 == 0` means `x & (1 == 0)`. Parentheses cost nothing and make intent obvious.

## Math functions

```cpp run
#include <cmath>
#include <iostream>

int main() {
    std::cout << std::sqrt(2.0) << ' ' << std::pow(2.0, 10) << ' ' << std::abs(-3.5) << ' '
              << std::floor(2.7) << ' ' << std::round(2.5) << ' ' << std::hypot(3.0, 4.0) << '\n';
}
```

:::exercise Seconds to clock time
Given `int total = 7384;` seconds, print hours, minutes and seconds (`2h 3m 4s`) using only `/` and `%`.
---solution---
```cpp run
#include <iostream>

int main() {
    int total = 7384;
    int h = total / 3600, m = (total % 3600) / 60, s = total % 60;
    std::cout << h << "h " << m << "m " << s << "s\n";
}
```
:::

::quiz
? What is `10 / 4` in C++?
+ 2
- 2.5
- 3
> Both operands are int, so it is integer division.
? Which expression correctly checks that x is between 1 and 9?
- 1 < x < 9
+ 1 < x && x < 9
- 1 < x and < 9
> C++ has no chained comparisons; combine two comparisons with &&.
::
