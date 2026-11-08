Programs make decisions and repeat work. C++'s tools for this will feel familiar from Python; mostly only the punctuation changes.

## if / else if / else

```cpp run
#include <iostream>

int main() {
    int score = 72;
    if (score >= 90) {
        std::cout << "A\n";
    } else if (score >= 70) {
        std::cout << "B\n";
    } else {
        std::cout << "C or below\n";
    }
}
```

The condition goes in **parentheses**, and the body in **braces**. With a single statement the braces are optional, but always write them: it prevents a famous class of bugs where a second line *looks* like it is inside the `if` but isn't.

:::python
`elif` becomes `else if`. There is no `pass`: an empty block is just `{}`. A condition can be any value that converts to `bool`: numbers are true when non-zero, like Python's truthiness, but an empty `std::string` or `std::vector` does **not** convert to `false`. Write `if (!v.empty())`.
:::

## The conditional operator

```cpp run
#include <iostream>

int main() {
    int n = 7;
    std::cout << (n % 2 == 0 ? "even" : "odd") << '\n';   // Python: "even" if n % 2 == 0 else "odd"
}
```

## switch: choosing between many fixed values

```cpp run
#include <iostream>

int main() {
    char cmd = 'q';
    switch (cmd) {
        case 'w': std::cout << "up\n"; break;
        case 's': std::cout << "down\n"; break;
        case 'q':
        case 'Q': std::cout << "quit\n"; break;   // two cases share one action
        default:  std::cout << "unknown\n";
    }
}
```

`switch` works on integers, characters and enums (not strings). **Each case needs `break`**, or execution “falls through” into the next case. Compilers warn about accidental fall-through.

## while and do-while

```cpp run
#include <iostream>

int main() {
    int n = 27, steps = 0;                // the Collatz sequence: does it always reach 1?
    while (n != 1) {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        ++steps;
    }
    std::cout << "27 reaches 1 after " << steps << " steps\n";
}
```

`do { ... } while (cond);` runs the body at least once before testing. It is handy for “ask until the input is valid”.

## for loops

The classic C++ `for` loop has three parts: **start; condition; step**.

```cpp run
#include <iostream>

int main() {
    for (int i = 0; i < 5; ++i) {          // Python: for i in range(5):
        std::cout << i << ' ';
    }
    std::cout << '\n';
    for (int i = 10; i > 0; i -= 3) std::cout << i << ' ';   // range(10, 0, -3)
    std::cout << '\n';
}
```

Read it as: create `i = 0`; while `i < 5`, run the body, then do `++i`.

## Range-based for: looping over a collection

```cpp run
#include <iostream>
#include <vector>

int main() {
    std::vector<int> primes{2, 3, 5, 7, 11};
    for (int p : primes) {                // Python: for p in primes:
        std::cout << p * p << ' ';
    }
    std::cout << '\n';
}
```

This is the loop you will write most often. Chapter 9 adds `for (const auto& x : things)`, which avoids copying big elements.

## break and continue

```cpp run
#include <iostream>

int main() {
    for (int n = 2; n < 50; ++n) {
        bool prime = true;
        for (int d = 2; d * d <= n; ++d) {
            if (n % d == 0) { prime = false; break; }   // leave the inner loop early
        }
        if (!prime) continue;                            // skip to the next n
        std::cout << n << ' ';
    }
    std::cout << '\n';
}
```

## Scope: where a variable exists

A variable lives from its declaration to the end of the enclosing `{ }` block. This is stricter than Python, where a variable created inside an `if` or loop still exists afterwards.

```cpp run expect=fail
#include <iostream>

int main() {
    for (int i = 0; i < 3; ++i) {
        int square = i * i;
    }
    std::cout << square << '\n';   // error: square only existed inside the loop
}
```

Small scopes are a feature: a variable can't be misused where it doesn't exist. Declare variables as late as possible, as close as possible to where they are used.

:::exercise FizzBuzz, then a multiplication table
1. Print the numbers 1 to 20, but print `Fizz` for multiples of 3, `Buzz` for multiples of 5 and `FizzBuzz` for multiples of both.
2. Print a 5 × 5 multiplication table using two nested loops.
---solution---
```cpp run
#include <iostream>

int main() {
    for (int i = 1; i <= 20; ++i) {
        if (i % 15 == 0) std::cout << "FizzBuzz ";
        else if (i % 3 == 0) std::cout << "Fizz ";
        else if (i % 5 == 0) std::cout << "Buzz ";
        else std::cout << i << ' ';
    }
    std::cout << "\n\n";
    for (int r = 1; r <= 5; ++r) {
        for (int c = 1; c <= 5; ++c) std::cout << r * c << '\t';
        std::cout << '\n';
    }
}
```
:::

::quiz
? What does this print? `for (int i = 0; i < 3; ++i) std::cout << i;`
+ 012
- 123
- 0123
> i starts at 0 and the loop stops when i < 3 becomes false.
? What happens without `break` at the end of a `case`?
- Compile error
+ Execution continues into the next case
- The switch ends anyway
> This is fall-through. It is occasionally useful, usually a bug.
::
