Real programs are spread over many files, built by a build system, and use third-party libraries. This chapter explains how the pieces fit: **headers**, **translation units**, the **linker**, and **CMake**. Most confusing C++ build errors make sense once you have this picture.

## Splitting code: headers and source files

A **header** (`.hpp` or `.h`) contains *declarations*: what exists. A **source file** (`.cpp`) contains *definitions*: how it works. Other files `#include` the header to use the code.

```cpp file=geometry.hpp
#pragma once                     // include this file's contents only once per translation unit

#include <vector>

namespace geo {                  // a namespace groups names and prevents clashes

struct Point { double x, y; };

double distance(Point a, Point b);                  // declarations only
double path_length(const std::vector<Point>& path);

}  // namespace geo
```

```cpp file=geometry.cpp
#include "geometry.hpp"          // quotes: a file from this project; <...>: a library header

#include <cmath>

namespace geo {

double distance(Point a, Point b) { return std::hypot(a.x - b.x, a.y - b.y); }

double path_length(const std::vector<Point>& path) {
    double total = 0;
    for (std::size_t i = 1; i < path.size(); ++i) total += distance(path[i - 1], path[i]);
    return total;
}

}  // namespace geo
```

```cpp run with=geometry.hpp,geometry.cpp title="main.cpp"
#include <iostream>

#include "geometry.hpp"

int main() {
    std::vector<geo::Point> route{{0, 0}, {3, 4}, {3, 10}};
    std::cout << "route length: " << geo::path_length(route) << '\n';
}
```

Built with `g++ -std=c++23 main.cpp geometry.cpp -o prog`.

## How it compiles: translation units and the linker

```text
 main.cpp  ──(#include geometry.hpp)──▶ compile ──▶ main.o     ┐
                                                                ├──▶ link ──▶ prog
 geometry.cpp ─(#include geometry.hpp)─▶ compile ──▶ geometry.o ┘
```

- Each `.cpp` file, plus everything it `#include`s, is a **translation unit**, compiled **independently** into an object file (`.o`).
- `main.o` contains a *reference* to `geo::path_length` but not its code. It only saw the declaration.
- The **linker** joins the object files and fills in every reference with the matching definition.

So the two classic build errors come from different stages:

| Error | Stage | Usual cause |
|---|---|---|
| `'foo' was not declared in this scope` | compiler | missing `#include` or typo |
| `undefined reference to 'foo'` | linker | declared but never defined, or you didn't compile/link the `.cpp` (or library) that defines it |
| `multiple definition of 'foo'` | linker | a non-inline function or variable *defined* in a header included by several `.cpp` files |

Here is the classic linker error: we declare `distance` but forget to compile `geometry.cpp`:

```cpp run expect=fail with=geometry.hpp
#include "geometry.hpp"

int main() {
    return static_cast<int>(geo::distance({0, 0}, {3, 4}));
}
```

The **one-definition rule (ODR)**: every function and variable must be defined exactly once in the whole program. Exceptions are `inline` functions, templates and class definitions, which may appear in every translation unit that includes them, provided all the copies are identical. That's why templates and small `inline` functions can live entirely in headers.

## Why separate compilation is worth it

When you change `geometry.cpp`, only that file is recompiled, and then the program is relinked. In a project with 5,000 files that is the difference between seconds and an hour. Headers should be small and include only what they need, because every file that includes a header is recompiled when the header changes.

## CMake: the standard build system

**CMake** describes your project once and generates build files for any platform: Makefiles, Ninja, Visual Studio or Xcode. Nearly every open-source C++ project uses it.

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(geo_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(geo geometry.cpp)                        # a library from our geometry code
target_include_directories(geo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(geo PRIVATE -Wall -Wextra)

add_executable(app main.cpp)                         # the program
target_link_libraries(app PRIVATE geo)               # link it against the library
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # configure: generate build files into ./build
cmake --build build -j                           # build in parallel
./build/app
```

Build types: `Debug` (`-g`, no optimisation), `Release` (`-O3 -DNDEBUG`), `RelWithDebInfo` (optimised with debug info, which is best for profiling). Add sanitizers to a debug build with `-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"`.

The modern CMake style is **target-based**: attach include directories, options and dependencies to targets (`target_...`) rather than setting global variables. `PUBLIC` means “I need this and so do my users”; `PRIVATE` means “only I need this”.

## Using libraries

| Way | How |
|---|---|
| **Header-only libraries** | copy the headers in, `#include` them. Examples: `nlohmann/json`, `fmt` (header-only mode), `stb_image`, `doctest`. |
| **System packages** | `sudo apt install libsdl2-dev`, then `find_package(SDL2 REQUIRED)` and `target_link_libraries(app PRIVATE SDL2::SDL2)` |
| **CMake FetchContent** | CMake downloads a library at configure time (shown below) |
| **Package managers** | **vcpkg** or **Conan**: like `pip` for C++ |

```cmake
include(FetchContent)
FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt GIT_TAG 10.2.1)
FetchContent_MakeAvailable(fmt)
target_link_libraries(app PRIVATE fmt::fmt)
```

**Static** libraries (`.a`, `.lib`) are copied into your program at link time. **Shared** libraries (`.so`, `.dll`, `.dylib`) are loaded when the program starts, so they can be updated separately and shared between programs. In CMake: `add_library(geo STATIC ...)` or `add_library(geo SHARED ...)`.

## A typical project layout

```text
myengine/
├── CMakeLists.txt
├── include/myengine/     public headers (what users #include)
│   └── math.hpp
├── src/                  implementation files
│   └── math.cpp
├── apps/                 executables that use the library
│   └── demo.cpp
├── tests/                unit tests (chapter 36)
│   └── test_math.cpp
└── third_party/          vendored dependencies
```

:::tip C++20 modules
C++20 introduced **modules** (`export module geo;` and `import geo;`) to replace headers with something faster and cleaner. Compiler and build-system support has matured through 2024–2026, and new projects can start using them with recent CMake, Clang, GCC 14+ and MSVC. You will still read header-based code for many years, so the model above remains essential.
:::

::quiz
? You get `undefined reference to 'geo::distance(...)'`. What's the most likely cause?
+ The .cpp file (or library) that defines it was not compiled or linked
- A missing #include
- A syntax error
> The compiler saw the declaration; the linker couldn't find a definition.
? Why can templates be defined in headers without multiple-definition errors?
+ The one-definition rule allows identical template (and inline) definitions in every translation unit
- Headers are compiled only once
- The linker ignores templates
> Ordinary non-inline function definitions in headers do cause multiple-definition errors.
? What does `#pragma once` prevent?
+ The same header being pasted twice into one translation unit
- The file being compiled
- Linker errors
> Older code uses include guards (#ifndef ... #define ... #endif) for the same purpose.
::
