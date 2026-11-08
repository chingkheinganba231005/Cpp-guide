A **pointer** is a variable whose value is an address. References were names for existing objects. Pointers are more flexible and more dangerous: they can be null, can be changed to point elsewhere, and can do arithmetic. Pointers are essential for data structures, for talking to C libraries, GPUs and operating systems, and for understanding everything else in C++.

## The two operators: & and *

```cpp run
#include <iostream>

int main() {
    int x = 42;
    int* p = &x;            // p holds the address of x. Read "int*" as "pointer to int".
    std::cout << "x  = " << x << '\n';
    std::cout << "&x = " << &x << '\n';
    std::cout << "p  = " << p << "  (the same address)\n";
    std::cout << "*p = " << *p << "  (follow the pointer: the value at that address)\n";
    *p = 7;                 // write through the pointer
    std::cout << "x is now " << x << '\n';
}
```

- `&x`: **address-of**. Gives the address where `x` lives.
- `*p`: **dereference**. Goes to the address stored in `p` and uses the object there.

:::eli5
A pointer is a slip of paper with a house number written on it. `&x` writes down x's house number. `*p` means “go to the house written on this slip”. You can copy the slip, cross out the number and write another, or leave it blank (`nullptr`). Visiting a blank slip, or a house that has been demolished, is the classic C++ disaster.
:::

Step through it:

<div data-widget="memstep" data-title="A pointer is an address stored in a variable">
<script type="application/json">
{"heap": false, "code": [
"int x = 42;",
"int y = 100;",
"int* p = &x;",
"*p = 7;",
"p = &y;",
"*p += 1;",
"p = nullptr;"],
"steps": [
{"line": 1, "note": "x is an int at some address.", "stack": [{"fn": "main", "vars": [{"name": "x", "type": "int", "addr": "0x7ffd20", "val": "42", "hi": true}]}]},
{"line": 2, "note": "y is another int, 4 bytes further on.", "stack": [{"fn": "main", "vars": [{"name": "x", "type": "int", "addr": "0x7ffd20", "val": "42"}, {"name": "y", "type": "int", "addr": "0x7ffd24", "val": "100", "hi": true}]}]},
{"line": 3, "note": "p is a variable too (8 bytes on a 64-bit machine). Its <b>value</b> is x's address.", "stack": [{"fn": "main", "vars": [{"name": "x", "type": "int", "addr": "0x7ffd20", "val": "42"}, {"name": "y", "type": "int", "addr": "0x7ffd24", "val": "100"}, {"name": "p", "type": "int*", "addr": "0x7ffd28", "val": "0x7ffd20", "to": "x", "hi": true}]}]},
{"line": 4, "note": "<code>*p = 7</code> follows the address and writes 7 into <b>x</b>.", "stack": [{"fn": "main", "vars": [{"name": "x", "type": "int", "addr": "0x7ffd20", "val": "7", "hi": true}, {"name": "y", "type": "int", "addr": "0x7ffd24", "val": "100"}, {"name": "p", "type": "int*", "addr": "0x7ffd28", "val": "0x7ffd20", "to": "x"}]}]},
{"line": 5, "note": "Unlike a reference, a pointer can be re-pointed: p now holds y's address.", "stack": [{"fn": "main", "vars": [{"name": "x", "type": "int", "addr": "0x7ffd20", "val": "7"}, {"name": "y", "type": "int", "addr": "0x7ffd24", "val": "100"}, {"name": "p", "type": "int*", "addr": "0x7ffd28", "val": "0x7ffd24", "to": "y", "hi": true}]}]},
{"line": 6, "note": "<code>*p += 1</code> now changes y.", "stack": [{"fn": "main", "vars": [{"name": "x", "type": "int", "addr": "0x7ffd20", "val": "7"}, {"name": "y", "type": "int", "addr": "0x7ffd24", "val": "101", "hi": true}, {"name": "p", "type": "int*", "addr": "0x7ffd28", "val": "0x7ffd24", "to": "y"}]}]},
{"line": 7, "note": "<b>nullptr</b> means “points to nothing”. Dereferencing it now (<code>*p</code>) would crash the program, so always check: <code>if (p) ...</code>", "stack": [{"fn": "main", "vars": [{"name": "x", "type": "int", "addr": "0x7ffd20", "val": "7"}, {"name": "y", "type": "int", "addr": "0x7ffd24", "val": "101"}, {"name": "p", "type": "int*", "addr": "0x7ffd28", "val": "nullptr", "hi": true}]}]}
]}
</script>
</div>

## nullptr: pointing at nothing

A pointer can hold the special value `nullptr`. Following a null pointer is undefined behaviour, and on most systems it crashes immediately with a **segmentation fault**:

```cpp run expect=crash
#include <iostream>

int main() {
    int* p = nullptr;
    std::cout << "about to dereference a null pointer..." << std::endl;
    std::cout << *p << '\n';
}
```

Always check a pointer that might be null before using it: `if (p != nullptr)`, or simply `if (p)`.

## Pointers to structs: the arrow operator

```cpp run
#include <iostream>
#include <string>

struct Player { std::string name; int hp; };

void heal(Player* p) {
    if (p) p->hp += 10;       // p->hp is shorthand for (*p).hp
}

int main() {
    Player hero{"Ada", 50};
    heal(&hero);
    heal(nullptr);            // safe: heal checks for null
    std::cout << hero.name << " has " << hero.hp << " hp\n";
}
```

## Pointers and arrays: pointer arithmetic

A pointer to an element of an array can move to the neighbouring elements. Adding 1 moves by **one element** (by `sizeof(T)` bytes), not one byte:

```cpp run
#include <iostream>

int main() {
    int a[5] = {10, 20, 30, 40, 50};
    int* p = a;                     // an array name decays to a pointer to its first element
    std::cout << *p << ' ' << *(p + 1) << ' ' << p[2] << '\n';    // p[i] is exactly *(p + i)
    std::cout << "p     = " << p << '\n' << "p + 1 = " << p + 1 << "  (4 bytes later)\n";
    for (int* q = a; q != a + 5; ++q) std::cout << *q << ' ';    // walk with a pointer
    std::cout << '\n';
}
```

This is how `std::vector`, strings and most low-level code work underneath: a pointer to the first element plus an index. `v.data()` gives you that pointer for a vector, which you will need when handing data to C libraries, GPUs or the operating system.

:::warn Array decay loses the size
When a C array is passed to a function it silently becomes a plain pointer, and the size is lost: inside the function `sizeof(arr)` is 8 (the size of a pointer). This is why modern C++ passes `std::span<T>` (a pointer plus a length, chapter 22) or `const std::vector<T>&`.
:::

## const and pointers

Two different things can be `const`: the data pointed to, or the pointer itself. Read the declaration right to left:

```cpp norun
const int* a;        // pointer to const int: can't change *a, can re-point a
int* const b = &x;   // const pointer to int: can change *b, can't re-point b
const int* const c = &x;   // neither
```

`const T*` parameters are common: “I need the address, but I won't modify what's there”.

## Pointer or reference?

| Use a **reference** when | Use a **pointer** when |
|---|---|
| the object must exist (no null) | “nothing” is a valid answer (`nullptr`) |
| it always refers to the same object | you need to re-point it (walking an array, linked structures) |
| passing parameters and returning aliases | talking to C APIs, the OS, GPUs, raw buffers |

And in modern C++, a raw pointer **never owns** memory (it is never responsible for freeing it). Owning pointers are the smart pointers of chapter 12.

## Pointers to pointers, and function pointers

`int** pp` is a pointer to a pointer to an int (you meet these in C APIs such as `main(int argc, char** argv)`). A **function pointer** holds the address of a function, which is how callbacks work in C. In C++ you usually use lambdas and `std::function` instead (chapter 19).

```cpp run
#include <iostream>

int add(int a, int b) { return a + b; }
int mul(int a, int b) { return a * b; }

int main(int argc, char** argv) {
    int (*op)(int, int) = add;          // a pointer to a function taking (int, int) and returning int
    std::cout << op(3, 4) << ' ';
    op = mul;
    std::cout << op(3, 4) << '\n';
    std::cout << "program name: " << argv[0] << ", argc = " << argc << '\n';
}
```

:::exercise Find the largest
Write `const int* find_max(const int* first, const int* last)` that walks from `first` up to (not including) `last` and returns a pointer to the largest element, or `nullptr` if the range is empty. Test it on a C array and on `v.data()` of a vector.
---solution---
```cpp run
#include <iostream>
#include <vector>

const int* find_max(const int* first, const int* last) {
    if (first == last) return nullptr;
    const int* best = first;
    for (const int* p = first + 1; p != last; ++p)
        if (*p > *best) best = p;
    return best;
}

int main() {
    int a[] = {3, 9, 2, 7};
    std::vector<int> v{-5, -1, -9};
    const int* m1 = find_max(a, a + 4);
    const int* m2 = find_max(v.data(), v.data() + v.size());
    std::cout << *m1 << " at index " << (m1 - a) << ", " << *m2 << '\n';
    std::cout << std::boolalpha << (find_max(a, a) == nullptr) << '\n';
}
```
:::

::quiz
? If `int* p = &x;`, what is `*p`?
+ The value of x
- The address of x
- The address of p
> * follows the address stored in p.
? `int a[4]; int* p = a;` What does `p + 2` point to?
+ a[2], which is 8 bytes after a[0]
- 2 bytes after a[0]
- a[1]
> Pointer arithmetic moves in whole elements, sizeof(int) = 4 bytes each.
? Which is true of a modern C++ raw pointer like `Player*`?
+ It may point at an object but should not own (free) it
- It always owns its object
- It can't be null
> Ownership belongs to smart pointers and containers; raw pointers just observe.
::
