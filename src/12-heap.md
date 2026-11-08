Stack variables die when their function returns, and the stack is small. When you need memory that **outlives** a function, that is **large**, or whose **size is decided at run time**, you ask for it from the **heap** (also called the free store). In modern C++ you rarely do this by hand: containers and smart pointers do it for you. But you must understand what they do, because every performance and memory bug you will ever chase lives here.

## new and delete

```cpp run
#include <iostream>

int main() {
    int* p = new int{42};          // ask the heap for space for one int; get its address
    std::cout << *p << " lives on the heap at " << p << '\n';
    *p += 1;
    delete p;                      // give it back. YOU must remember to do this.
    p = nullptr;                   // good habit: p no longer points anywhere valid

    int n = 5;                     // a run-time size
    double* arr = new double[n]{1, 2, 3, 4, 5};   // an array on the heap
    double sum = 0;
    for (int i = 0; i < n; ++i) sum += arr[i];
    std::cout << "sum " << sum << '\n';
    delete[] arr;                  // arrays need delete[] (not delete)
}
```

Step through what happens:

<div data-widget="memstep" data-title="The heap: you ask, you must give back">
<script type="application/json">
{"code": [
"int* make_counter() {",
"    int* c = new int{0};",
"    return c;",
"}",
"int main() {",
"    int* a = make_counter();",
"    *a += 5;",
"    int* b = a;",
"    delete a;",
"    // *b = 1;  would be use-after-free!",
"    int* leak = new int{99};",
"}"],
"steps": [
{"line": 5, "note": "main starts with an empty heap.", "stack": [{"fn": "main", "vars": []}], "heap": []},
{"line": 2, "note": "<b>new int{0}</b> asks the heap for 4 bytes, writes 0 there, and returns the address. The pointer c lives on the stack; the int lives on the heap.", "stack": [{"fn": "main", "vars": []}, {"fn": "make_counter", "vars": [{"name": "c", "type": "int*", "addr": "0x7ffd10", "val": "0x55a0", "to": "heap int", "hi": true}]}], "heap": [{"name": "int", "addr": "0x55a0", "val": "0", "hi": true}]},
{"line": 6, "note": "make_counter returns. Its frame (and c) is gone, but the <b>heap int survives</b>: heap memory has no automatic lifetime. a receives the address.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int*", "addr": "0x7ffd30", "val": "0x55a0", "to": "heap int", "hi": true}]}], "heap": [{"name": "int", "addr": "0x55a0", "val": "0"}]},
{"line": 7, "note": "Writing through a.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int*", "addr": "0x7ffd30", "val": "0x55a0", "to": "heap int"}]}], "heap": [{"name": "int", "addr": "0x55a0", "val": "5", "hi": true}]},
{"line": 8, "note": "Copying a pointer copies the <b>address</b>, not the int. Now two pointers point at one heap object. Which one is responsible for deleting it? This question, <b>ownership</b>, is the heart of C++ memory management.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int*", "addr": "0x7ffd30", "val": "0x55a0", "to": "heap int"}, {"name": "b", "type": "int*", "addr": "0x7ffd38", "val": "0x55a0", "to": "heap int", "hi": true}]}], "heap": [{"name": "int", "addr": "0x55a0", "val": "5"}]},
{"line": 9, "note": "<b>delete a</b> returns the memory to the heap. But b still holds the old address: b is now a <b>dangling pointer</b>. Using it is undefined behaviour.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int*", "addr": "0x7ffd30", "val": "0x55a0 (freed)", "dead": true}, {"name": "b", "type": "int*", "addr": "0x7ffd38", "val": "0x55a0 (dangling!)", "leak": true}]}], "heap": [{"name": "int", "addr": "0x55a0", "val": "freed", "dead": true}]},
{"line": 11, "note": "A new allocation, never deleted…", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int*", "addr": "0x7ffd30", "val": "0x55a0 (freed)", "dead": true}, {"name": "b", "type": "int*", "addr": "0x7ffd38", "val": "0x55a0 (dangling!)", "leak": true}, {"name": "leak", "type": "int*", "addr": "0x7ffd40", "val": "0x55c0", "to": "heap int", "hi": true}]}], "heap": [{"name": "int", "addr": "0x55c0", "val": "99", "hi": true}]},
{"line": 12, "note": "main ends and the stack pointer <b>leak</b> disappears, but the heap int it pointed to does not. Nothing refers to it any more, so it can never be freed: a <b>memory leak</b>. Leak inside a loop and your program eats all the RAM.", "stack": [], "heap": [{"name": "int", "addr": "0x55c0", "val": "99 (leaked)", "leak": true}]}
]}
</script>
</div>

## The three classic heap bugs

| Bug | What happens | Python equivalent |
|---|---|---|
| **Memory leak**: forgetting `delete` | memory is never returned; long-running programs grow until they die | impossible (garbage collector) |
| **Use after free / dangling pointer**: using memory after `delete` | reads garbage, corrupts other data, crashes later somewhere unrelated | impossible |
| **Double free**: `delete` twice | corrupts the heap's bookkeeping; often crashes | impossible |

Plus `new[]` with `delete` (instead of `delete[]`), and buffer overflows: writing past the end of an allocation. These bugs are behind a large share of the world's security vulnerabilities, and they are why modern C++ style tries hard to make manual `new`/`delete` unnecessary.

Here is a use-after-free caught by **AddressSanitizer** (chapter 13), which adds checks to the program when you compile with `-fsanitize=address`:

```cpp run flags="-fsanitize=address -g -O0" expect=crash
#include <iostream>

int main() {
    int* p = new int{5};
    delete p;
    std::cout << *p << '\n';     // use after free
}
```

Read the report from the top: `heap-use-after-free`, then *where* the bad read happened, where the memory was freed, and where it was allocated. Without the sanitizer this program would probably print 5, or garbage, and seem fine. That is the scary part.

## Why it's hard: ownership and exceptions

Manual `delete` is fragile even when you are careful. An early `return` or an exception (chapter 20) jumps past your `delete`:

```cpp norun
void process() {
    Image* img = new Image(4096, 4096);
    if (!img->load("photo.png")) return;     // leak! delete is skipped
    analyse(*img);                           // if analyse throws: leak!
    delete img;
}
```

The fix is not “be more careful”. The fix is to give the memory an **owner object** whose destructor frees it automatically, whatever path the code takes. This idea is called **RAII**, and it is the most important idea in C++ (chapters 13 and 15). You have already been using it: `std::vector` and `std::string` allocate on the heap and free automatically when they go out of scope.

```cpp run
#include <iostream>
#include <vector>

int main() {
    int n = 1'000'000;
    std::vector<double> data(n, 1.5);   // a heap allocation, owned by the vector
    std::cout << data.size() << " doubles on the heap at " << data.data() << '\n';
}   // the vector's destructor frees the memory here, automatically. No leak is possible.
```

## The rules of modern C++

1. **Prefer the stack.** Local variables and members by value: no allocation at all.
2. **Use containers** for collections: `std::vector`, `std::string`, `std::map` …
3. **Use smart pointers** (`std::unique_ptr`, `std::make_unique`) when you need a single heap object.
4. **Never write `new` and `delete` in ordinary code.** They belong inside the implementation of containers, smart pointers and custom allocators (chapter 27), where one carefully-tested class owns them.

## The cost of heap allocation

A heap allocation is not free: the allocator has to find a suitable block and keep its records up to date. For small objects an allocate-and-free pair typically costs 10 to 50 nanoseconds, depending on the allocator and on how fragmented the heap is. That is fast for a human and slow for a CPU, which can do dozens of additions per nanosecond:

```cpp run
#include <chrono>
#include <iostream>
#include <memory>

// Optimisers are clever: if a result is never used, they delete the work, even new/delete pairs.
// This empty assembly statement tells GCC/Clang "assume this pointer is used", so the work stays.
// (Benchmark libraries call it DoNotOptimize.)
inline void keep(const void* p) { asm volatile("" : : "g"(p) : "memory"); }

int main() {
    constexpr int N = 2'000'000;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) { int x = i; keep(&x); }                       // stack
    auto t1 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) { auto p = std::make_unique<int>(i); keep(p.get()); }   // heap, every time
    auto t2 = std::chrono::steady_clock::now();
    auto ns = [&](auto a, auto b) { return std::chrono::duration<double, std::nano>(b - a).count() / N; };
    std::cout << "stack: " << ns(t0, t1) << " ns per iteration\n";
    std::cout << "heap:  " << ns(t1, t2) << " ns per iteration (allocate + free)\n";
}
```

Without `keep`, the optimiser notices that nothing is ever done with the values and removes both loops entirely, and the timings come out as zero. Measuring performance honestly takes care. Chapter 26 covers benchmarking properly.

This is why games, trading engines and real-time audio avoid allocating in their hot loops. They allocate up front and reuse memory, or use custom allocators (chapter 27).

::quiz
? A function allocates with `new` and returns early without `delete`. What is this?
+ A memory leak
- A dangling pointer
- Fine: the heap is freed when the function returns
> Heap memory has no automatic lifetime; only stack memory is freed on return.
? After `delete p;`, what is p?
+ A dangling pointer that still holds the old address
- Automatically nullptr
- Still valid until the function ends
> delete does not change p. Set it to nullptr yourself, or better, don't use raw owning pointers.
? What is the modern C++ advice for new and delete in everyday code?
+ Don't write them; use containers and smart pointers
- Always pair each new with a delete on the next line
- Use malloc instead
> Owner objects (RAII) free memory automatically on every path.
::
