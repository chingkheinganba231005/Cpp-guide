You already know a little Python. Python is wonderful for trying ideas quickly. But the programs this course builds towards are game engines, GPU kernels, machine-learning libraries and trading systems that react in microseconds. Almost all of them are written in **C++**. PyTorch, TensorFlow and NumPy's fast parts, Unreal Engine, Chrome, most game consoles, every major trading firm's core systems and the CUDA ecosystem are C++ underneath. This chapter explains why, and what changes when you move from Python.

## What a computer actually runs

A CPU understands only **machine code**: very simple instructions such as “add these two numbers”, “copy these 8 bytes from memory” and “jump to that instruction if the result was zero”. Every language has to end up there somehow.

:::eli5
Python is like a live interpreter at a meeting: every sentence you say is translated on the spot, every time. C++ is like translating a whole book once, carefully, before anyone reads it. Reading the translated book is then very fast, because nobody is translating any more.
:::

- **Python** is *interpreted*. When you run `python script.py`, another program (the interpreter) reads your code line by line and performs each step. Every `a + b` involves checking what types `a` and `b` are, finding the right addition routine, and creating a new object for the result.
- **C++** is *compiled*. A program called the **compiler** reads all your code before it runs. It checks it, and translates it into machine code for your CPU. The result is an executable file that runs on its own, with no interpreter. `a + b` on two integers becomes a single CPU instruction.

That is the main reason C++ is often 10 to 100 times faster than plain Python for the same algorithm.

## Static types: decided before the program runs

In Python a variable can hold anything, and it can change:

```python
x = 5        # x refers to an int
x = "hello"  # now it refers to a str: Python does not mind
```

In C++ every variable has a **type** fixed when you write the program, and the compiler enforces it:

```cpp run expect=fail title="The compiler refuses to put text into an integer"
int main() {
    int x = 5;
    x = "hello";   // error: a string is not an int
}
```

At first this feels strict. It is actually a gift: whole families of bugs are caught **before the program ever runs**. Because the compiler knows exactly what every value is, it can generate exactly the right machine instructions.

:::python
In Python, `x` is a *name tag* attached to an object that lives somewhere else, and the object carries its own type. In C++, `int x` is **a box of exactly 4 bytes, right here**, that can only ever hold an integer. Chapter 8 makes this concrete.
:::

## “Close to the machine”, and why that matters

C++ lets you control things Python decides for you:

| You control | Why it matters |
|---|---|
| **Where data lives in memory** and how it is laid out | Modern CPUs are fast only when data arrives in the right order (chapter 26). Game engines and ML libraries are designed around this. |
| **When memory is allocated and freed** | No garbage collector pausing your program, which is essential for games (60 frames per second) and trading (microseconds). |
| **Exactly which CPU instructions are used** (SIMD, atomics) | The same matrix multiplication can run 30× faster or more (chapter 31), and threads can share data without locks (chapter 34). |
| **Talking to hardware and the OS directly** | GPU kernels, drivers, network stacks, operating systems. |

The price is responsibility. C++ trusts you, and some mistakes, such as reading memory you no longer own, are not caught automatically. Part II teaches you to avoid them, and the tools that find them.

## “Zero-cost abstractions”

C++'s design promise is: *what you don't use, you don't pay for, and what you do use, you couldn't hand-code any better.* You can write high-level code with classes, templates, containers and lambdas, and the compiler turns it into machine code as fast as hand-written low-level code. That combination of high-level design with low-level speed is why C++ is still the language for performance-critical systems, 40 years after it was created.

```cpp run title="High-level code, low-level speed: sum a million numbers"
#include <iostream>
#include <numeric>
#include <vector>
#include <chrono>

int main() {
    std::vector<double> v(1'000'000, 0.5);                 // a million numbers, each 0.5
    auto t0 = std::chrono::steady_clock::now();
    double total = std::accumulate(v.begin(), v.end(), 0.0);  // a high-level algorithm
    auto t1 = std::chrono::steady_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "total = " << total << " in about " << (us < 5000 ? "a millisecond or less" : "a few milliseconds") << '\n';
}
```

The same loop in plain Python (`sum` over a list of floats) takes roughly 5 to 10 milliseconds. NumPy is fast precisely because its inner loops are written in C and C++.

## What C++ is used for, and where this course goes

- **Games and engines**: Unreal, Godot, most AAA engines. Chapters 28–29.
- **Graphics and GPUs**: renderers, CUDA kernels, game physics. Chapters 29, 31, 32.
- **Machine learning infrastructure**: PyTorch's core (ATen), ONNX Runtime, llama.cpp. Chapters 30–31.
- **Finance**: exchanges, market makers and low-latency trading engines. Chapters 33–34.
- **Systems**: browsers, databases, compilers, operating-system components. Chapter 35.

## C++ versions

C++ is updated every three years: C++11 (the big modernisation), 14, 17, 20 and 23. This course teaches **modern C++ (C++20/23)** from the start. Many online tutorials still teach a 1990s style full of manual memory management, and you will learn why to avoid it.

::quiz
? What does a C++ compiler produce?
- A Python file
+ A program in machine code that the CPU can run directly
- An interpreter
> The compiler translates all your source code into machine code before the program runs. No interpreter is needed afterwards.
? Why does `int x = 5; x = "hello";` fail in C++?
- Because strings are not allowed in C++
+ Because x has the fixed type int, and the compiler checks types before running
- It doesn't fail; it prints hello
> Types are static in C++: decided when you write the code and checked by the compiler.
::
