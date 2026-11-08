Professional C++ is not just code that works today. It is code that **stays** correct as it changes. This chapter covers the habits and tools that make that possible: automated tests, strict compiler settings, sanitizers in continuous integration, static analysis and formatting.

## Unit tests

A **unit test** is a small program that checks one piece of code against known answers. To see that there's no magic in a test framework, here is a tiny one built with the tools from this course:

```cpp run
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct Test { std::string name; std::function<void()> fn; };
std::vector<Test>& registry() { static std::vector<Test> r; return r; }
struct Register { Register(std::string n, std::function<void()> f) { registry().push_back({std::move(n), std::move(f)}); } };
struct Failure { std::string what; };

#define TEST(name) void name(); static Register reg_##name(#name, name); void name()
#define CHECK(cond) do { if (!(cond)) throw Failure{std::string(#cond) + " (line " + std::to_string(__LINE__) + ")"}; } while (0)

// --- the code under test ---
long long gcd(long long a, long long b) { return b == 0 ? (a < 0 ? -a : a) : gcd(b, a % b); }
double mean(const std::vector<double>& v) { double s = 0; for (double x : v) s += x; return v.empty() ? 0 : s / v.size(); }

// --- the tests ---
TEST(gcd_basic) { CHECK(gcd(12, 18) == 6); CHECK(gcd(17, 5) == 1); }
TEST(gcd_edge_cases) { CHECK(gcd(0, 5) == 5); CHECK(gcd(-12, 18) == 6); }
TEST(mean_values) { CHECK(std::abs(mean({1, 2, 3, 4}) - 2.5) < 1e-12); CHECK(mean({}) == 0); }
TEST(deliberately_failing) { CHECK(gcd(10, 4) == 3); }          // shows what a failure looks like

int main() {
    int failed = 0;
    for (const auto& t : registry()) {
        try { t.fn(); std::cout << "[ PASS ] " << t.name << '\n'; }
        catch (const Failure& f) { ++failed; std::cout << "[ FAIL ] " << t.name << ": " << f.what << '\n'; }
    }
    std::cout << registry().size() - failed << " passed, " << failed << " failed\n";
}
```

Real projects use a framework that adds rich assertions, fixtures, filtering and reporting:

| Framework | Notes |
|---|---|
| **GoogleTest** | the most widely used; with GoogleMock for mocks |
| **Catch2** | expressive `REQUIRE(a == b)` that prints both values; BDD-style sections |
| **doctest** | a single header, very fast to compile; tests can live next to the code |
| **Google Benchmark** | the matching tool for performance tests (chapter 26) |

```cpp norun
// tests/test_math.cpp with GoogleTest
#include <gtest/gtest.h>
#include "mylib/math.hpp"

TEST(Gcd, Basic) {
    EXPECT_EQ(gcd(12, 18), 6);
    EXPECT_EQ(gcd(0, 5), 5);
}
```

```cmake
# CMake: fetch GoogleTest and register the tests with CTest
include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(unit_tests tests/test_math.cpp)
target_link_libraries(unit_tests PRIVATE mylib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(unit_tests)          # then: ctest --test-dir build --output-on-failure
```

## What to test

- **Normal cases** and **edge cases**: empty inputs, one element, maximum values, negative numbers, zero.
- **Invariants**: after any sequence of operations, does the class's rule still hold? (For example, the order book is never crossed: the best bid is always below the best ask.)
- **Against a reference**: compare an optimised implementation with a simple one on many random inputs. That is exactly how chapter 31 checked the fast matmul and chapter 33 checked the fast order book. This kind of **differential test** is the most powerful test for performance code.
- **Regressions**: every bug you fix gets a test that would have caught it.

**Fuzzing** goes further: a tool such as libFuzzer (`-fsanitize=fuzzer`) generates millions of inputs, guided by which code paths they reach, and combined with ASan and UBSan it finds crashes no human would think to test. Parsers and network code should always be fuzzed.

## Strict compilation

```bash
# a good set of warnings for GCC and Clang, treated as errors in your own code
-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wnon-virtual-dtor \
-Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wnull-dereference -Wdouble-promotion -Werror
```

MSVC uses `/W4 /permissive- /WX`. Apply `-Werror` to your own targets, not to third-party code.

## Continuous integration

Run everything automatically on every push, for example with GitHub Actions: build in several configurations, and run the tests with sanitizers:

```yaml
# .github/workflows/ci.yml
name: ci
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        config: [Release, Debug-asan, Debug-tsan]
    steps:
      - uses: actions/checkout@v4
      - name: Configure
        run: |
          case "${{ matrix.config }}" in
            Release)    FLAGS="" ; TYPE=Release ;;
            Debug-asan) FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" ; TYPE=Debug ;;
            Debug-tsan) FLAGS="-fsanitize=thread" ; TYPE=Debug ;;
          esac
          cmake -S . -B build -DCMAKE_BUILD_TYPE=$TYPE -DCMAKE_CXX_FLAGS="$FLAGS"
      - run: cmake --build build -j
      - run: ctest --test-dir build --output-on-failure
```

## Static analysis and formatting

- **clang-tidy**: hundreds of checks for bugs and modern style (`bugprone-*`, `modernize-*`, `performance-*`, `cppcoreguidelines-*`). Run it on every change.
- **cppcheck** and compiler analysers (`-fanalyzer` in GCC, the Clang Static Analyzer) find bugs without running the code.
- **clang-format**: automatic, consistent formatting from a `.clang-format` file, so code review never argues about spaces.
- **Include-what-you-use**: keeps headers minimal, which keeps builds fast (chapter 21).

## Guidelines to know

- The **C++ Core Guidelines** (by Bjarne Stroustrup and Herb Sutter) are the community's collected advice. Many of this course's rules (RAII, no owning raw pointers, `const` by default) come from there.
- Your team's style guide: Google's, LLVM's or your company's. Consistency matters more than any particular choice.

::quiz
? You optimised a function. What is the strongest quick test?
+ Compare its output with the simple reference version on many random inputs
- Run it once and look at the output
- Check that it compiles with -O3
> Differential testing catches subtle mistakes in fast code.
? Why run tests with sanitizers in CI?
+ So memory errors, UB and data races are caught automatically on every change
- Sanitizers make tests faster
- Because release builds can't run tests
> Many bugs don't crash on their own; sanitizers make them visible.
::
