To write C++ you need three things: a **compiler** (turns source code into a program), an **editor** (where you type), and a **terminal** (where you run the compiler and your program). This chapter sets them up and walks through what happens when you compile.

## The fastest start: Compiler Explorer in your browser

Before installing anything, open [Compiler Explorer (godbolt.org)](https://godbolt.org). Choose C++ on the left, type code, and it compiles instantly. Turn on “Output” to run the program. Every runnable example in this course has an **Open in Compiler Explorer** link that loads the example there, ready to run and edit. It is the best way to experiment while you read.

## Installing a compiler

There are three major compilers: **GCC** (`g++`), **Clang** (`clang++`) and Microsoft's **MSVC**. All are excellent and free.

| Your system | What to install | Check it worked |
|---|---|---|
| **Windows** | [Visual Studio Community](https://visualstudio.microsoft.com/) with “Desktop development with C++” (MSVC), *or* WSL (Windows Subsystem for Linux) and then the Linux steps | `cl` in the “Developer Command Prompt”, or `g++ --version` in WSL |
| **macOS** | Run `xcode-select --install` in Terminal (installs Clang) | `clang++ --version` |
| **Linux** (Ubuntu/Debian) | `sudo apt install build-essential gdb cmake` | `g++ --version` |

You want a version from 2023 or later (GCC 13+, Clang 17+, Visual Studio 2022) so that C++23 features work.

For the editor, **VS Code** with Microsoft's C/C++ extension (or the clangd extension) is a good default. CLion and Visual Studio are full IDEs (editor + compiler + debugger in one).

## Your first compile, step by step

Save this as `hello.cpp`:

```cpp run
#include <iostream>

int main() {
    std::cout << "Hello from C++!\n";
}
```

Then in a terminal, in the same folder:

```bash
g++ -std=c++23 -Wall -Wextra hello.cpp -o hello    # compile: creates a program called "hello"
./hello                                            # run it (on Windows: hello.exe)
```

What the flags mean:

| Flag | Meaning |
|---|---|
| `-std=c++23` | use the C++23 standard (newest features) |
| `-Wall -Wextra` | turn on (almost) all **warnings**: the compiler tells you about suspicious code. Always use them. |
| `-o hello` | name of the output program (default would be `a.out`) |
| `-O2` | optimise for speed (use for anything you measure) |
| `-g` | include debugging information (for the debugger, chapter 13) |

:::python
In Python you run `python hello.py` and that's it. In C++ there are two steps: **compile** once, then **run** as often as you like. If you change the code, you must compile again. Forgetting to recompile and wondering why nothing changed is everyone's first C++ mistake.
:::

## What happens when you compile

```text
 hello.cpp ──▶ preprocessor ──▶ compiler ──▶ assembler ──▶ linker ──▶ hello (executable)
            (#include pasted in)  (C++ → assembly)  (→ machine code .o)  (joins your code with the libraries)
```

1. **Preprocessor**: handles lines starting with `#`. `#include <iostream>` literally pastes in the declarations for input/output.
2. **Compiler**: checks your code (types, syntax) and translates it to assembly language for your CPU.
3. **Assembler**: turns assembly into machine code, an *object file*.
4. **Linker**: combines your object files with the libraries you use (like the code behind `std::cout`) into one executable.

Most of the time you don't think about these steps. They matter when something goes wrong: a *compile error* comes from step 2, and a *linker error* (“undefined reference to …”) comes from step 4. Chapter 21 explains linker errors properly.

:::deep
You can see the assembly your code turns into: in Compiler Explorer it is the right-hand panel. Locally, `g++ -O2 -S hello.cpp` writes `hello.s`. You don't need to read assembly to learn C++, but peeking at it later in the course shows exactly what “zero-cost” means.
:::

## Build systems, briefly

For one file, typing the `g++` command is fine. Real projects have hundreds of files, and you use a **build system**. The standard one is **CMake**, which chapter 21 teaches. You will see `CMakeLists.txt` files in almost every open-source C++ project.

::quiz
? You edited hello.cpp and ran ./hello again, but the old message still prints. Why?
- C++ caches output
+ You didn't recompile; ./hello is still the old program
- The terminal needs restarting
> The executable is a separate file made by the compiler. Change the source, and you must compile again.
? Which flags should you almost always use while learning?
+ -Wall -Wextra
- -O3 only
- -w (no warnings)
> Warnings point at code that compiles but is probably wrong. Turn them on and read them.
::
