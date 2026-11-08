In Python, mistakes raise exceptions with a clear message. Many C++ mistakes do something stranger: they cause **undefined behaviour** (UB). This chapter explains what that means, why C++ has it, and the tools that make these bugs findable. **AddressSanitizer, UndefinedBehaviorSanitizer and a debugger** will save you weeks.

## What “undefined behaviour” means

The C++ standard lists certain operations whose result is simply *not defined*: reading outside an array, using freed memory, signed integer overflow, dereferencing null, reading an uninitialised variable, data races between threads, and more. If your program does one of these, **the standard places no requirements at all** on what happens. It might:

- appear to work (the worst outcome, because it hides the bug),
- crash immediately, or much later somewhere unrelated,
- give wrong answers only when optimisation is on, or only on another compiler,
- be exploited by an attacker.

Why would a language allow this? Because checking every array access, every addition and every pointer would slow every program down. C++ makes **you** responsible, and in return the optimiser can assume UB never happens, which lets it generate much faster code.

:::eli5
UB is like a contract that says “you will never drive on the left”. Because of that promise, the road builders didn't put up barriers. If you break the promise, nobody can say what will happen. Maybe nothing, maybe a crash a mile later.
:::

## UB that changes under optimisation

Here the compiler assumes signed integers never overflow, so it decides that `i + 1 > i` is always true:

```cpp run
#include <climits>
#include <iostream>

bool next_is_bigger(int i) { return i + 1 > i; }     // overflow is UB, so the compiler assumes it can't happen

int main() {
    std::cout << std::boolalpha << next_is_bigger(5) << ' ' << next_is_bigger(INT_MAX) << '\n';
}
```

Mathematically `INT_MAX + 1` would wrap to a negative number, so you might expect `false`. With `-O2` GCC simply returns `true`. The program “works”, but it isn't doing the arithmetic you think it is. **UndefinedBehaviorSanitizer** catches the actual overflow:

```cpp run flags="-fsanitize=undefined -g -O0"
#include <climits>
#include <iostream>

int main() {
    int big = INT_MAX;
    int volatile one = 1;
    int r = big + one;          // signed overflow
    std::cout << r << '\n';
}
```

## AddressSanitizer: your best friend

Compile with `-fsanitize=address -g` (GCC and Clang; Visual Studio has `/fsanitize=address`). The program runs about 2× slower and reports memory errors precisely, with the line numbers of the bad access, the allocation and the free.

A buffer overflow on the heap:

```cpp run flags="-fsanitize=address -g -O0" expect=crash
#include <vector>

int main() {
    std::vector<int> v(4);
    int* p = v.data();
    p[4] = 1;                   // one past the end
    return p[0];
}
```

A dangling pointer into a vector that reallocated (the bug chapter 7 warned about):

```cpp run flags="-fsanitize=address -g -O0" expect=crash
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{1, 2, 3};
    int& first = v[0];              // a reference into the vector's heap block
    for (int i = 0; i < 100; ++i) v.push_back(i);   // the vector grows: its block moves
    std::cout << first << '\n';     // first refers to the OLD, freed block
}
```

A dangling `string_view` (chapter 6's warning):

```cpp run flags="-fsanitize=address -g -O0" expect=crash
#include <iostream>
#include <string>
#include <string_view>

std::string_view first_word(std::string s) {   // s is a local copy...
    return std::string_view(s).substr(0, s.find(' '));    // ...and we return a view into it
}

int main() {
    std::string_view w = first_word("supercalifragilistic words longer than the small-string buffer");
    std::cout << w << '\n';
}
```

:::tip The habit that matters most
Build and run your tests with `-fsanitize=address,undefined -g` during development. Most experienced C++ teams do this in their automated tests. Bugs that would take days to find in a debugger show up instantly with a line number.
:::

## Other tools in the same family

| Tool | Flag | Finds |
|---|---|---|
| AddressSanitizer (ASan) | `-fsanitize=address` | out-of-bounds, use-after-free, double free, leaks |
| UndefinedBehaviorSanitizer (UBSan) | `-fsanitize=undefined` | overflow, bad shifts, null deref, misaligned access … |
| ThreadSanitizer (TSan) | `-fsanitize=thread` | data races between threads (chapter 25) |
| MemorySanitizer (Clang) | `-fsanitize=memory` | reads of uninitialised memory |
| Valgrind | `valgrind ./prog` | memory errors without recompiling (slower) |
| Library debug mode | `-D_GLIBCXX_DEBUG` (GCC), `_ITERATOR_DEBUG_LEVEL` (MSVC) | bad iterators and indices in the standard library |

## Warnings are free bug reports

Compilers can see many bugs before the program runs. Turn on the strictest useful warnings and treat them as errors in your own code:

```bash
g++ -std=c++23 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror main.cpp
```

```cpp run expect=fail flags="-Wshadow -Wconversion -Werror"
int scale(int value, double factor) {
    int value2 = value * factor;      // -Wconversion: double to int loses information
    {
        int value = 3;                // -Wshadow: hides the parameter named value
        return value2 + value;
    }
}

int main() { return scale(2, 1.5); }
```

## Using a debugger

A debugger runs your program under control: pause at a line, inspect variables, step line by line, and see the call stack when it crashes. Compile with `-g -O0` (debug info, no optimisation), then:

```bash
gdb ./prog            # or lldb ./prog on macOS
(gdb) break main.cpp:12     # stop at line 12
(gdb) run
(gdb) next                  # run the next line
(gdb) step                  # step into a function call
(gdb) print total           # show a variable
(gdb) bt                    # backtrace: which functions called which
(gdb) continue
```

When a program crashes with a segmentation fault, `gdb ./prog` then `run` then `bt` shows exactly which line crashed and how you got there. Your IDE (VS Code, CLion, Visual Studio) offers the same features with buttons, and is worth learning on day one.

## Debugging habits that work

1. **Reproduce** the bug with the smallest input you can.
2. **Read** the first error or sanitizer report completely.
3. **Rebuild with sanitizers** before reaching for the debugger.
4. **Bisect**: comment out half, or check halfway with a print, until the faulty line is cornered.
5. **Write a test** that fails because of the bug, then fix it, so it can never come back (chapter 36).

::quiz
? A program with undefined behaviour prints the right answer on your machine. What can you conclude?
+ Nothing: it may break with another compiler, other flags or other input
- It is correct
- The compiler fixed it
> UB has no guaranteed behaviour, including appearing to work.
? Which flag finds use-after-free and buffer overflows at run time?
+ -fsanitize=address
- -O3
- -Wall
> AddressSanitizer instruments every memory access.
? Your program crashes with a segmentation fault. What is the fastest first step?
+ Rebuild with -fsanitize=address -g and run it again
- Add prints everywhere
- Rewrite the program
> The sanitizer report names the faulty line, the allocation and the free.
::
