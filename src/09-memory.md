This chapter is the turning point of the course. Python hides memory from you completely. In C++, understanding memory is what lets you write fast programs, and what stops you writing crashing ones. The ideas are simple once you can picture them.

## Memory is a long row of numbered bytes

:::eli5
Picture a street with billions of tiny houses in a single row. Each house holds one **byte** (a number from 0 to 255), and each has a **house number**, its **address**. A variable is a name for a few neighbouring houses: an `int` occupies 4 houses in a row, a `double` 8.
:::

```cpp run
#include <iostream>

int main() {
    int a = 42;
    double b = 3.5;
    char c = 'z';
    std::cout << "a lives at " << &a << " and takes " << sizeof a << " bytes\n";
    std::cout << "b lives at " << &b << " and takes " << sizeof b << " bytes\n";
    std::cout << "c lives at " << static_cast<void*>(&c) << " and takes " << sizeof c << " byte\n";
}
```

`&a` means “the address of `a`”. Addresses are printed in hexadecimal (base 16, the `0x` prefix). Yours will differ: operating systems deliberately randomise where programs are loaded, as a security measure.

:::python
In Python, `id(x)` gives you something similar, but you can never *use* it. In C++ an address is an ordinary value: you can store it, pass it around and follow it. That is what pointers are (chapter 10).
:::

## Where variables live: the stack

When a function is called, the program reserves a block of memory for that call's local variables, called its **stack frame**. When the function returns, the frame is released instantly, all at once. Frames are stacked on top of one another like plates, hence the name.

Step through this program and watch the frames appear and disappear:

<div data-widget="memstep" data-title="Function calls and stack frames">
<script type="application/json">
{"heap": false, "code": [
"int square(int x) {",
"    int result = x * x;",
"    return result;",
"}",
"",
"int main() {",
"    int a = 3;",
"    int b = square(a);",
"    int c = square(b);",
"    std::cout << c;",
"}"],
"steps": [
{"line": 6, "note": "The program starts: the operating system calls <b>main</b>, and main gets a stack frame with room for a, b and c (not yet set).", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "?"}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "?"}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "?"}]}]},
{"line": 7, "note": "<code>a = 3</code> is written into a's 4 bytes.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "3", "hi": true}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "?"}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "?"}]}]},
{"line": 1, "note": "Calling <b>square(a)</b> pushes a new frame on top. The parameter x is a <b>copy</b> of a's value.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "3"}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "?"}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "?"}]}, {"fn": "square", "vars": [{"name": "x", "type": "int", "addr": "0x7ffcf0", "val": "3", "hi": true}, {"name": "result", "type": "int", "addr": "0x7ffcf4", "val": "?"}]}]},
{"line": 2, "note": "result is computed inside square's own frame.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "3"}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "?"}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "?"}]}, {"fn": "square", "vars": [{"name": "x", "type": "int", "addr": "0x7ffcf0", "val": "3"}, {"name": "result", "type": "int", "addr": "0x7ffcf4", "val": "9", "hi": true}]}]},
{"line": 8, "note": "<b>return</b> copies 9 back to the caller, and square's whole frame is popped: x and result no longer exist. b receives 9.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "3"}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "9", "hi": true}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "?"}]}]},
{"line": 1, "note": "A second call gets a <b>fresh</b> frame, at the same place in memory the first one used. Nothing from the first call survives.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "3"}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "9"}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "?"}]}, {"fn": "square", "vars": [{"name": "x", "type": "int", "addr": "0x7ffcf0", "val": "9", "hi": true}, {"name": "result", "type": "int", "addr": "0x7ffcf4", "val": "?"}]}]},
{"line": 9, "note": "square returns 81 and its frame is popped again.", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "3"}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "9"}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "81", "hi": true}]}]},
{"line": 10, "note": "c is printed.", "out": "81", "stack": [{"fn": "main", "vars": [{"name": "a", "type": "int", "addr": "0x7ffd10", "val": "3"}, {"name": "b", "type": "int", "addr": "0x7ffd14", "val": "9"}, {"name": "c", "type": "int", "addr": "0x7ffd18", "val": "81"}]}]},
{"line": 11, "note": "main returns, its frame is released, and the program ends. Every stack variable was cleaned up automatically, with no garbage collector.", "out": "81", "stack": []}
]}
</script>
</div>

The real program, printing real addresses, shows the same thing: both calls to `square` use the same memory, *below* main's frame (on most systems the stack grows towards lower addresses):

```cpp run
#include <iostream>

int square(int x) {
    int result = x * x;
    std::cout << "  square(" << x << "): x at " << &x << '\n';
    return result;
}

int main() {
    int a = 3;
    std::cout << "main: a at " << &a << '\n';
    int b = square(a);
    int c = square(b);
    std::cout << "c = " << c << '\n';
}
```

Why the stack is wonderful:

- **Allocation is nearly free**: making room for a frame is one CPU instruction that moves the “stack pointer”.
- **Cleanup is automatic and immediate**: when a function returns, its locals vanish.
- **It is cache-friendly**: the top of the stack is almost always in the CPU's fastest memory.

Its limits: it is small (typically 1–8 MB per thread), and a local variable dies when its function returns. For data that must outlive a function, or is very large, or whose size is only known at run time, we need the **heap** (chapter 11). `std::vector` and `std::string` already use the heap for you behind the scenes.

## An object is its bytes

A C++ variable *is* the bytes, not a reference to an object elsewhere. A struct (chapter 14) is just its members laid out one after another:

```cpp run
#include <cstddef>
#include <iostream>

struct Particle {
    float x, y, z;     // 3 × 4 bytes
    int id;            // 4 bytes
};

int main() {
    Particle p{1.0f, 2.0f, 3.0f, 7};
    std::cout << "sizeof(Particle) = " << sizeof(Particle) << '\n';
    std::cout << "x at offset " << offsetof(Particle, x) << ", y at " << offsetof(Particle, y)
              << ", z at " << offsetof(Particle, z) << ", id at " << offsetof(Particle, id) << '\n';
    Particle many[1000];
    std::cout << "1000 particles = " << sizeof many << " bytes, back to back\n";
    (void)p;
}
```

A million Python objects each carry a header of 16 or more bytes, live scattered across memory, and sit behind pointers. A million `Particle`s in C++ take exactly 16 MB in one contiguous run. That difference, multiplied by the CPU's appetite for contiguous data, is a big part of why C++ is fast (chapter 26).

## Alignment and padding

The CPU prefers values at addresses that are multiples of their size: a 4-byte `int` at an address divisible by 4, a `double` at one divisible by 8. To make this true, the compiler inserts invisible **padding** bytes inside structs:

```cpp run predict
#include <iostream>

struct Bad  { char a; double b; char c; };   // 1 + (7 padding) + 8 + 1 + (7 padding)
struct Good { double b; char a; char c; };   // 8 + 1 + 1 + (6 padding)

int main() {
    std::cout << sizeof(Bad) << ' ' << sizeof(Good) << '\n';
}
```

Ordering struct members from largest to smallest often saves memory. In a game with millions of entities, or a trading system with millions of orders, that is real speed.

## The memory map of a running program

| Region | What lives there | Lifetime |
|---|---|---|
| **Code (text)** | your compiled machine instructions | the whole run |
| **Static / global data** | global variables, `static` variables, string literals | the whole run |
| **Heap** | memory you request at run time (`new`, and inside `vector`, `string` …) | until you (or the owning object) give it back |
| **Stack** | local variables and function call frames | until the enclosing block or function ends |

```cpp run
#include <iostream>
#include <vector>

int global_counter = 0;                       // static data

int main() {
    int local = 1;                            // stack
    std::vector<int> v{1, 2, 3};              // the vector object itself: stack; its elements: heap
    static int calls = 0;                     // static data
    std::cout << "code   (main)        " << reinterpret_cast<void*>(&main) << '\n'
              << "static (global)      " << &global_counter << '\n'
              << "static (calls)       " << &calls << '\n'
              << "heap   (v's data)    " << v.data() << '\n'
              << "stack  (local)       " << &local << '\n';
}
```

Look at the addresses: code and static data sit together low in the address space, the heap starts just above them, and the stack is far away at the top. The heap grows upwards and the stack grows downwards, towards each other.

## A first taste of danger

Because a stack variable dies when its function returns, keeping its address afterwards is a bug: the address now points into memory that will be reused. The compiler warns:

```cpp run expect=fail flags="-Werror=return-local-addr"
int* make_number() {
    int n = 42;
    return &n;       // returning the address of a local variable: it dies right here
}

int main() {
    int* p = make_number();
    return *p;
}
```

(This example turns that warning into an error with `-Werror`. Without it, the program compiles and its behaviour is undefined.) Chapters 10–13 show how to handle addresses and lifetimes safely.

::quiz
? What happens to a function's local variables when it returns?
+ They are destroyed and their stack space is reused
- They are kept until the program ends
- The garbage collector frees them later
> The frame is popped. There is no garbage collector in C++.
? Why can reordering struct members change sizeof?
+ Padding is inserted to keep members aligned
- The compiler compresses structs
- It cannot change sizeof
> char, double, char needs padding after each char; double, char, char packs tighter.
? Where do the elements of a std::vector live?
- On the stack, inside the vector
+ On the heap; the vector object holds a pointer to them
- In static memory
> The small vector object (pointer, size, capacity) is local; its elements are allocated on the heap.
::
