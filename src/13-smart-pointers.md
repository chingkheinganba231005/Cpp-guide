A **smart pointer** is an object that holds a pointer *and owns* the memory it points to: when the smart pointer is destroyed, it frees the memory. With smart pointers, leaks, double frees and most dangling pointers become impossible by construction. This is C++'s answer to Python's garbage collector, and it frees memory deterministically, at a known moment, with no pauses.

## Ownership: the question every C++ design answers

For every heap object, ask: **who is responsible for freeing it?**

- **Exactly one owner** → `std::unique_ptr<T>`. By far the most common case.
- **Several owners, and the object should live until the last one is done** → `std::shared_ptr<T>`.
- **Someone who wants to look but not own** → a raw pointer `T*`, a reference `T&`, or `std::weak_ptr<T>` (for observing something owned by `shared_ptr`).

## std::unique_ptr: the single owner

```cpp run
#include <iostream>
#include <memory>
#include <string>

struct Texture {
    std::string name;
    explicit Texture(std::string n) : name(std::move(n)) { std::cout << "  load " << name << '\n'; }
    ~Texture() { std::cout << "  free " << name << '\n'; }      // destructor: runs when the object dies
};

int main() {
    std::cout << "start\n";
    {
        auto grass = std::make_unique<Texture>("grass.png");   // allocate and own
        std::cout << "  using " << grass->name << '\n';
    }   // grass goes out of scope here: the Texture is freed automatically
    std::cout << "after the block\n";
    auto rock = std::make_unique<Texture>("rock.png");
    std::cout << "end of main\n";
}   // rock freed here
```

Look at the order of the output: each texture is freed exactly when its owner's scope ends. No `delete` anywhere, and no way to forget one.

A `unique_ptr` is exactly the size of a raw pointer, and using it costs nothing extra at run time. There is no reason to use an owning raw pointer instead.

## unique_ptr cannot be copied, only moved

“Unique” means one owner. Copying would create two owners, so the compiler forbids it:

```cpp run expect=fail
#include <memory>

int main() {
    auto a = std::make_unique<int>(5);
    auto b = a;          // error: copying would mean two owners
}
```

To hand ownership to someone else, **move** it with `std::move`. The source becomes empty (null):

```cpp run
#include <iostream>
#include <memory>
#include <vector>

struct Enemy {
    int hp;
    explicit Enemy(int h) : hp(h) {}
    ~Enemy() { std::cout << "  enemy with " << hp << " hp destroyed\n"; }
};

std::unique_ptr<Enemy> spawn(int hp) {
    return std::make_unique<Enemy>(hp);          // ownership leaves the function with the return value
}

int main() {
    std::vector<std::unique_ptr<Enemy>> enemies;  // the vector owns the enemies
    auto e = spawn(30);
    enemies.push_back(std::move(e));              // ownership moves into the vector; e is now empty
    enemies.push_back(spawn(50));
    std::cout << std::boolalpha << "e is empty: " << (e == nullptr) << '\n';
    std::cout << enemies.size() << " enemies\n";
    enemies.erase(enemies.begin());               // removing an element destroys that enemy
    std::cout << "clearing the rest\n";
}
```

Chapter 15 explains moves properly. For now: `std::move(x)` means “I'm handing x's contents over; x will be left empty”.

## Passing smart pointers to functions

A common confusion with a simple rule. If a function only *uses* the object, pass a reference or raw pointer. It doesn't care who owns it:

```cpp norun
void draw(const Texture& t);                  // just uses it (most common)
void maybe_draw(const Texture* t);            // just uses it, may be null
void take_ownership(std::unique_ptr<Texture> t);   // the function becomes the owner (caller must std::move)
```

Call `draw(*grass)` or `maybe_draw(grass.get())`. `.get()` returns the raw, non-owning pointer.

## std::shared_ptr: shared ownership with a reference count

`shared_ptr` keeps a **reference count** next to the object: the number of `shared_ptr`s currently owning it. Copying increments the count; destroying a copy decrements it. When it reaches zero, the object is freed. (This is how Python itself manages most memory.)

```cpp run
#include <iostream>
#include <memory>
#include <string>

struct Mesh {
    std::string name;
    explicit Mesh(std::string n) : name(std::move(n)) {}
    ~Mesh() { std::cout << "  mesh " << name << " freed\n"; }
};

int main() {
    std::shared_ptr<Mesh> tree_model;
    {
        auto m = std::make_shared<Mesh>("oak");
        std::cout << "count after make_shared: " << m.use_count() << '\n';
        tree_model = m;                                  // a second owner
        std::cout << "count after copy: " << m.use_count() << '\n';
    }   // m dies, count drops to 1: the mesh survives
    std::cout << "count after the block: " << tree_model.use_count() << '\n';
    tree_model.reset();                                  // last owner lets go: freed now
    std::cout << "end\n";
}
```

`shared_ptr` is convenient but not free: the count is updated with atomic (thread-safe) operations, and every copy touches it. Reach for `unique_ptr` first. Use `shared_ptr` only when ownership really is shared (an asset cache used by many game objects, or a node in a graph whose lifetime has no single owner).

## Cycles and std::weak_ptr

Two objects that hold `shared_ptr`s to each other keep each other alive forever: neither count reaches zero. That is a leak, even with smart pointers. Break cycles with `std::weak_ptr`, which observes without owning:

```cpp run
#include <iostream>
#include <memory>

struct Node {
    std::shared_ptr<Node> next;      // owns the next node
    std::weak_ptr<Node> prev;        // only observes the previous one: no cycle
    int value;
    explicit Node(int v) : value(v) {}
    ~Node() { std::cout << "  node " << value << " freed\n"; }
};

int main() {
    auto a = std::make_shared<Node>(1);
    auto b = std::make_shared<Node>(2);
    a->next = b;
    b->prev = a;
    if (auto p = b->prev.lock()) std::cout << "b's prev is " << p->value << '\n';   // lock() gives a temporary shared_ptr, or null if gone
    std::cout << "leaving main\n";
}
```

Change `prev` to a `shared_ptr` and run it: neither node is ever freed.

## Smart pointers are one case of RAII

`unique_ptr` is one example of the central C++ idiom, **RAII** (“Resource Acquisition Is Initialisation”): *tie every resource to the lifetime of an object.* The constructor acquires the resource and the destructor releases it. The same pattern manages files (`std::fstream` closes itself), locks (`std::lock_guard` unlocks itself, chapter 25), GPU buffers, sockets and database connections. Chapter 15 shows how to write your own RAII classes.

You can even give `unique_ptr` a custom “deleter”, to manage resources from C libraries:

```cpp run
#include <cstdio>
#include <iostream>
#include <memory>

int main() {
    // fclose is called automatically when the unique_ptr dies, even if an exception is thrown
    auto closer = [](std::FILE* fp) { std::fclose(fp); };
    std::unique_ptr<std::FILE, decltype(closer)> f(std::fopen("demo.txt", "w"), closer);
    if (f) std::fputs("written through a RAII-managed C FILE*\n", f.get());
    std::cout << "file " << (f ? "opened" : "failed") << '\n';
}
```

:::exercise A scene graph
Make a `struct SceneNode { std::string name; std::vector<std::unique_ptr<SceneNode>> children; }` with a destructor that prints the name. Build a small tree (root → two children → one grandchild), then let `root` go out of scope. In what order are the nodes destroyed, and why is no `delete` needed?
---solution---
```cpp run
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct SceneNode {
    std::string name;
    std::vector<std::unique_ptr<SceneNode>> children;
    explicit SceneNode(std::string n) : name(std::move(n)) {}
    ~SceneNode() { std::cout << "destroy " << name << '\n'; }
    SceneNode* add(std::string n) { children.push_back(std::make_unique<SceneNode>(std::move(n))); return children.back().get(); }
};

int main() {
    auto root = std::make_unique<SceneNode>("root");
    SceneNode* car = root->add("car");
    car->add("wheel");
    root->add("tree");
}
```
Each node's destructor body runs first, then its members are destroyed, including the vector, which destroys the children in order. Ownership flows down the tree, so destroying the root cleans up everything.
:::

::quiz
? You need one owner for a heap object. What should you use?
+ std::unique_ptr via std::make_unique
- std::shared_ptr
- a raw pointer with new and delete
> unique_ptr expresses single ownership at zero cost.
? Why does `auto b = a;` fail when a is a unique_ptr?
+ Copying would create two owners; you must std::move it
- unique_ptr has no assignment
- It needs .get()
> Ownership can be transferred (moved), not duplicated.
? Two objects hold shared_ptrs to each other. What happens when everything else lets go?
+ They leak: each keeps the other's count above zero
- They are freed normally
- The program crashes
> Break the cycle with a weak_ptr on one side.
::
