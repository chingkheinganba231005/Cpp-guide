Every C++ program has the same skeleton. Once you can read it, and read the compiler's complaints when you get it wrong, you can learn everything else one piece at a time.

## The shape of a program

```cpp run
#include <iostream>    // 1. bring in the input/output part of the standard library

int main() {           // 2. every program starts running here
    std::cout << "Hello, world!\n";   // 3. print a line
    return 0;          // 4. tell the operating system "everything went fine"
}
```

Line by line:

1. `#include <iostream>` pastes in the declarations of the standard **input/output stream** library. Without it, the compiler has never heard of `std::cout`.
2. `int main()` defines a **function** called `main`. The operating system calls `main` when your program starts. `int` means it returns an integer to the operating system.
3. `std::cout << "..."` sends text to the **c**haracter **out**put (your terminal). `std::` says “from the standard library”. `\n` is a newline.
4. `return 0;` ends `main` and reports success. In `main` only, you may leave it out and C++ assumes `return 0`.

Three rules that apply everywhere:

- **Statements end with a semicolon** `;`.
- **Curly braces `{ }` group code** into blocks. Indentation is for humans only; the compiler ignores it.
- **C++ is case-sensitive**: `Main` is not `main`.

:::python
Python uses indentation and line ends to structure code. C++ uses `{ }` and `;`. You can write a whole C++ program on one line (please don't), and indenting badly changes nothing for the compiler. Indent anyway: four spaces, like Python.
:::

## Printing several things

`<<` can be chained. Numbers are printed as you'd expect:

```cpp run
#include <iostream>

int main() {
    std::cout << "2 + 3 = " << 2 + 3 << '\n';
    std::cout << "A line" << '\n' << "and another" << '\n';
    std::cout << "pi is about " << 3.14159 << '\n';
}
```

`'\n'` (single quotes) is a single character, and `"\n"` (double quotes) is a piece of text containing one character. Both print a newline. You will also see `std::endl` in older code; it prints a newline *and* forces the output to be written immediately, which is slower. Prefer `'\n'`.

## Comments

```cpp norun
// a comment to the end of the line (like Python's #)

/* a comment that can
   span several lines */
```

## Reading compiler errors (a skill, not a punishment)

You will see thousands of compiler errors. Reading them calmly is one of the most valuable skills in C++. Here is a classic: a missing semicolon.

```cpp run expect=fail
#include <iostream>

int main() {
    std::cout << "Hello"
    std::cout << "World\n";
}
```

How to read it:

- `main.cpp:4:25` means **file : line : column**. Go there first.
- The message says `expected ';' before 'std'`: the compiler reached `std` on the next line while still waiting for the `;` that ends line 4.
- **Fix the first error first.** One mistake can cause a cascade of confusing errors below it; often fixing the first makes the rest disappear.

Another classic: forgetting the `#include`.

```cpp run expect=fail
int main() {
    std::cout << "no include!\n";
}
```

GCC even suggests the fix: `'std::cout' is defined in header '<iostream>'; this is probably fixable by adding '#include <iostream>'`. Modern compilers are helpful if you actually read the whole message.

:::tip
Copy the first error line into a search engine if it confuses you. Thousands of people have hit exactly the same message.
:::

## `using namespace std;`, and why this course avoids it

Many tutorials start with `using namespace std;` so they can write `cout` instead of `std::cout`. In real projects it is discouraged: the standard library has thousands of names (`count`, `distance`, `size` …) and pulling them all in causes name clashes. This course writes `std::` explicitly. It is five characters, and it tells the reader where things come from.

:::exercise Personal banner
Write a program that prints your name in a box made of `*` characters, like:
```text
**************
* Your Name  *
**************
```
---solution---
```cpp run
#include <iostream>

int main() {
    std::cout << "**************\n";
    std::cout << "* Your Name  *\n";
    std::cout << "**************\n";
}
```
:::

::quiz
? Where does a C++ program start running?
- At the first line of the file
+ In the function main
- At the first #include
> The operating system calls main(). Code outside functions (like #include) is instructions to the compiler, not steps that run.
? The compiler reports 12 errors. What should you do first?
+ Fix the first error, then compile again
- Fix the last error
- Fix them all at once
> Later errors are often consequences of the first one.
::
