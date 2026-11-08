This chapter covers the ideas that most distinguish C++ from every garbage-collected language: **destructors**, **RAII**, and **copy and move semantics**. Master these and you understand how `std::vector`, `std::unique_ptr` and every resource-owning class in every engine work.

## Destructors: code that runs when an object dies

A **destructor** is named `~ClassName()` and runs automatically when an object's lifetime ends: when a local variable goes out of scope, when a heap object is deleted, or when a containing object is destroyed. Objects are destroyed in the **reverse order** of construction.

```cpp run
#include <iostream>
#include <string>

struct Tracer {
    std::string name;
    explicit Tracer(std::string n) : name(std::move(n)) { std::cout << "construct " << name << '\n'; }
    ~Tracer() { std::cout << "destroy   " << name << '\n'; }
};

int main() {
    Tracer a{"a"};
    {
        Tracer b{"b"};
        Tracer c{"c"};
        std::cout << "-- leaving inner block --\n";
    }
    Tracer d{"d"};
    std::cout << "-- leaving main --\n";
}
```

## RAII: tie every resource to an object's lifetime

**RAII** (Resource Acquisition Is Initialisation): acquire a resource in a constructor and release it in the destructor. The resource might be memory, a file, a lock, a socket, a GPU buffer or a database connection. Because destructors run on *every* exit path (normal return, early `return`, `break`, an exception), cleanup can't be forgotten.

```cpp run
#include <cstdio>
#include <iostream>
#include <stdexcept>

class File {                           // a minimal RAII wrapper around a C FILE*
public:
    File(const char* path, const char* mode) : f_(std::fopen(path, mode)) {
        if (!f_) throw std::runtime_error("cannot open file");
        std::cout << "opened " << path << '\n';
    }
    ~File() { std::fclose(f_); std::cout << "closed\n"; }
    void write(const char* s) { std::fputs(s, f_); }

    File(const File&) = delete;              // copying a file handle makes no sense: forbid it
    File& operator=(const File&) = delete;
private:
    std::FILE* f_;
};

void save(bool fail) {
    File out("log.txt", "w");
    out.write("first line\n");
    if (fail) throw std::runtime_error("something went wrong");   // the file is still closed!
    out.write("second line\n");
}

int main() {
    try { save(true); } catch (const std::exception& e) { std::cout << "caught: " << e.what() << '\n'; }
}
```

The file is closed before the exception reaches `main`. This is **why modern C++ doesn't need `finally` or `with`**: every object is its own `with` block.

:::python
Python's `with open(...) as f:` closes the file at the end of the block. C++ RAII does the same for **every** object, automatically, without extra syntax. Garbage-collected languages can't do this for memory, because they don't know *when* an object becomes garbage.
:::

## Copying: what happens when you write `b = a`

For a class like `Vec2`, copying the members is exactly right. But consider a class that owns heap memory through a raw pointer. The compiler's default copy duplicates the **pointer**, not the data, and both objects then think they own the same block:

```cpp run flags="-fsanitize=address -g -O0" expect=crash
#include <cstddef>

class Buffer {                    // BROKEN: owns memory but uses the default copy
public:
    explicit Buffer(std::size_t n) : data_(new float[n]), size_(n) {}
    ~Buffer() { delete[] data_; }
private:
    float* data_;
    std::size_t size_;
};

int main() {
    Buffer a(100);
    Buffer b = a;     // copies the pointer: a and b now share one block
}                     // b's destructor frees it, then a's destructor frees it AGAIN
```

Double free, caught by AddressSanitizer. The fix is to define what copying *means* for this class.

## The rule of three (and five)

If a class needs a custom **destructor**, it almost certainly needs a custom **copy constructor** and **copy assignment** too (the rule of three). Since C++11 add **move constructor** and **move assignment** (the rule of five). Here is a correct, complete version. It is long, and the next section shows why you'll rarely need to write one:

```cpp run
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <utility>

class Buffer {
public:
    explicit Buffer(std::size_t n) : data_(new float[n]{}), size_(n) {}
    ~Buffer() { delete[] data_; }

    Buffer(const Buffer& o) : data_(new float[o.size_]), size_(o.size_) {          // copy constructor: deep copy
        std::copy(o.data_, o.data_ + size_, data_);
        std::cout << "  copy (" << size_ << " floats)\n";
    }
    Buffer& operator=(const Buffer& o) {                                           // copy assignment
        if (this != &o) { Buffer tmp(o); swap(tmp); }                             // copy-and-swap: simple and safe
        return *this;
    }
    Buffer(Buffer&& o) noexcept : data_(std::exchange(o.data_, nullptr)), size_(std::exchange(o.size_, 0)) {   // move constructor: steal
        std::cout << "  move (no allocation)\n";
    }
    Buffer& operator=(Buffer&& o) noexcept { Buffer tmp(std::move(o)); swap(tmp); return *this; }   // move assignment
    void swap(Buffer& o) noexcept { std::swap(data_, o.data_); std::swap(size_, o.size_); }

    std::size_t size() const { return size_; }
    float& operator[](std::size_t i) { return data_[i]; }
private:
    float* data_;
    std::size_t size_;
};

Buffer make_big() { Buffer b(1'000'000); b[0] = 1; return b; }   // returned without any copy (copy elision)

int main() {
    Buffer a(1000);
    std::cout << "b = a:\n";
    Buffer b = a;                 // copy: a stays valid
    std::cout << "c = std::move(a):\n";
    Buffer c = std::move(a);      // move: c steals a's block; a is left empty
    std::cout << "a.size() after move = " << a.size() << '\n';
    std::cout << "d = make_big():\n";
    Buffer d = make_big();        // no copy, no move printed: the object is built directly in d
    std::cout << "d.size() = " << d.size() << ", c.size() = " << c.size() << ", b.size() = " << b.size() << '\n';
}
```

## Move semantics: steal instead of copy

A **move** transfers the *resources* of an object that is about to be thrown away, instead of duplicating them. Moving a `std::vector` of a million elements copies three pointers instead of a million numbers.

The language tells the two cases apart by **value category**:

- An **lvalue** has a name and an address, and will still be used later: `a`, `v[3]`, `*p`. It must be **copied**, because someone may still need it.
- An **rvalue** is a temporary about to disappear: `make_big()`, `x + y`, `std::string("hi")`. It can be **moved from**, because nobody can observe it afterwards.
- `T&&` is an **rvalue reference**: a parameter that binds only to rvalues. That is how the move constructor is selected.
- `std::move(x)` does **not** move anything. It is a cast that says “treat `x` as an rvalue: I promise not to use its value again”. The move constructor then does the stealing.

```cpp run
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string big(1000, 'x');
    std::vector<std::string> v;
    v.push_back(big);               // copies: big is an lvalue, still needed
    v.push_back(std::move(big));    // moves: big's buffer is handed over
    std::cout << "big is now " << (big.empty() ? "empty" : "not empty") << " (valid but unspecified: only reassign or destroy it)\n";
    v.push_back(std::string(500, 'y'));   // a temporary: moved automatically
    std::cout << v.size() << " strings, sizes " << v[0].size() << ' ' << v[1].size() << ' ' << v[2].size() << '\n';
}
```

After a move, the source is in a “valid but unspecified” state: you may destroy it or assign a new value to it, but don't rely on its contents.

:::deep Why `noexcept` on moves matters
When `std::vector` grows, it moves its elements to the new block, but only if the element's move constructor is marked `noexcept`. Otherwise it copies them, to keep a strong guarantee if a move throws halfway. Forgetting `noexcept` on a custom move constructor can silently turn every reallocation into a full copy.
:::

## The rule of zero: the best rule

Writing the five special functions by hand is error-prone. The better approach: **don't own raw resources directly.** Make every member something that already manages itself (`std::vector`, `std::string`, `std::unique_ptr`), and then you write **none** of the five. The compiler-generated versions do the right thing:

```cpp run
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Mesh {                       // rule of zero: no destructor, no copy/move code
public:
    Mesh(std::string name, std::size_t n) : name_(std::move(name)), vertices_(n * 3, 0.0f) {}
    std::size_t vertex_count() const { return vertices_.size() / 3; }
private:
    std::string name_;
    std::vector<float> vertices_;      // manages its own memory
};

class Model {                      // owns a mesh it doesn't want copied: unique_ptr makes it move-only
public:
    explicit Model(std::unique_ptr<Mesh> m) : mesh_(std::move(m)) {}
    const Mesh& mesh() const { return *mesh_; }
private:
    std::unique_ptr<Mesh> mesh_;
};

int main() {
    Mesh a("cube", 8);
    Mesh b = a;                                   // a correct deep copy, generated by the compiler
    Model m(std::make_unique<Mesh>("tree", 5000));
    Model m2 = std::move(m);                      // movable; copying would not compile
    std::cout << b.vertex_count() << ' ' << m2.mesh().vertex_count() << '\n';
}
```

**Rule of zero** for application code. **Rule of five** only inside the few low-level classes that wrap a raw resource, and each of those should manage exactly one resource. This is exactly how the standard library itself is built.

::quiz
? When does a local object's destructor run?
+ When its scope ends, on every path out of it, including exceptions
- When the garbage collector runs
- Only if you call it
> That guarantee is what makes RAII work.
? What does `std::move(x)` do by itself?
+ Nothing at run time: it casts x to an rvalue so a move constructor or assignment can be chosen
- It copies x
- It frees x
> The move constructor or assignment does the actual transfer.
? Your class has a std::vector member and a std::string member. Which special functions should you write?
+ None (rule of zero)
- All five
- Just the destructor
> The members already manage their resources correctly.
::
