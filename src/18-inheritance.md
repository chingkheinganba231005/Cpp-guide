**Inheritance** lets a class build on another class. **Polymorphism** lets you treat different types through one common interface and have the right code run for each. These are the classic object-oriented tools. C++ supports them fully, and modern C++ also teaches when *not* to use them.

## Inheritance: a derived class extends a base class

```cpp run
#include <iostream>
#include <string>

class Animal {
public:
    explicit Animal(std::string name) : name_(std::move(name)) {}
    const std::string& name() const { return name_; }
    void sleep() const { std::cout << name_ << " is sleeping\n"; }
private:
    std::string name_;
};

class Dog : public Animal {           // Dog IS-AN Animal: it gets name() and sleep()
public:
    explicit Dog(std::string name) : Animal(std::move(name)) {}   // construct the base part first
    void fetch() const { std::cout << name() << " fetches the ball\n"; }
};

int main() {
    Dog d("Rex");
    d.sleep();       // inherited
    d.fetch();       // its own
}
```

## Virtual functions: the right method for the real type

The real power comes from **virtual** functions. A pointer or reference to the base class can refer to any derived object, and calling a virtual function runs the **derived** class's version:

```cpp run
#include <iostream>
#include <memory>
#include <numbers>
#include <vector>

class Shape {
public:
    virtual ~Shape() = default;              // essential: see below
    virtual double area() const = 0;         // "= 0": pure virtual. Shape is abstract (an interface)
    virtual const char* name() const { return "shape"; }
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_(r) {}
    double area() const override { return std::numbers::pi * r_ * r_; }
    const char* name() const override { return "circle"; }
private:
    double r_;
};

class Rect : public Shape {
public:
    Rect(double w, double h) : w_(w), h_(h) {}
    double area() const override { return w_ * h_; }
    const char* name() const override { return "rectangle"; }
private:
    double w_, h_;
};

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(1.0));
    shapes.push_back(std::make_unique<Rect>(2.0, 3.0));
    double total = 0;
    for (const auto& s : shapes) {
        std::cout << s->name() << ": " << s->area() << '\n';   // the right area() for each real type
        total += s->area();
    }
    std::cout << "total " << total << '\n';
}
```

- `virtual` in the base says “derived classes may override this”.
- `override` in the derived class asks the compiler to check that you really are overriding something (it catches typos in names or parameters). Always write it.
- `= 0` makes the function **pure virtual**: the base has no implementation, and the class becomes **abstract** (you cannot create a plain `Shape`). An abstract class with only pure virtual functions is an **interface**.

:::python
In Python *every* method call is “virtual”: it is looked up on the object at run time (duck typing). In C++ a call is resolved at compile time, which is faster, **unless** the function is `virtual`. You choose where you want run-time flexibility.
:::

## Why base classes need a virtual destructor

When you delete a derived object through a base pointer (as `unique_ptr<Shape>` does), the destructor must be virtual, or only the base part is destroyed:

```cpp run flags="-fsanitize=address -g -O0" expect=crash
#include <memory>
#include <vector>

struct Base { ~Base() {} };                         // NOT virtual: a bug
struct Derived : Base { std::vector<int> big = std::vector<int>(1000); };

int main() {
    std::unique_ptr<Base> p = std::make_unique<Derived>();
}   // only ~Base runs; Derived's vector is never freed (and the size passed to delete is wrong)
```

Rule: **a class with any virtual function gets `virtual ~ClassName() = default;`.**

## How virtual calls work: the vtable

Each class with virtual functions has a hidden table of function pointers, the **vtable**. Each object stores a hidden pointer to its class's table (8 bytes). A virtual call loads that pointer, finds the right slot and jumps:

```cpp run
#include <iostream>

struct Plain { double x; void f() {} };
struct Poly  { double x; virtual void f() {} virtual ~Poly() = default; };

int main() {
    std::cout << "sizeof(Plain) = " << sizeof(Plain) << ", sizeof(Poly) = " << sizeof(Poly) << " (8 extra bytes: the vtable pointer)\n";
}
```

A virtual call costs a couple of memory loads and an indirect jump, and it usually prevents inlining. That's negligible for a handful of calls, but it matters in a loop over millions of particles. That is one reason game engines and ML libraries prefer data-oriented designs (chapters 26 and 28) and templates (chapter 18) in their hot paths.

## Alternatives to inheritance

Inheritance is often overused. Consider these first:

| Tool | Use when |
|---|---|
| **Composition** (a class *has* members) | reusing behaviour: a `Car` has an `Engine`; it is not an engine |
| **`std::variant`** (chapter 22) | a value is one of a **fixed, known** set of types: `Circle` or `Rect`. No heap allocation, no vtable. |
| **Templates** (chapter 18) | the type is known at compile time and you want zero overhead |
| **`std::function` / lambdas** | “a behaviour” is all you need: a callback, a strategy |
| **Virtual interfaces** | the set of types is **open** (plugins, user extensions) or chosen at run time |

Here is the same shapes example with `std::variant`, with no pointers and no virtual functions:

```cpp run
#include <iostream>
#include <numbers>
#include <variant>
#include <vector>

struct Circle { double r; };
struct Rect { double w, h; };
using Shape = std::variant<Circle, Rect>;

double area(const Circle& c) { return std::numbers::pi * c.r * c.r; }
double area(const Rect& r) { return r.w * r.h; }

int main() {
    std::vector<Shape> shapes{Circle{1.0}, Rect{2.0, 3.0}};      // stored by value, contiguously
    double total = 0;
    for (const auto& s : shapes) total += std::visit([](const auto& x) { return area(x); }, s);
    std::cout << "total " << total << '\n';
}
```

## A few more facts

- **protected** members are visible to derived classes but not to outsiders. Use them sparingly; prefer private data with protected functions.
- **final** on a class prevents further derivation; on a virtual function it prevents further overriding. It can also help the compiler optimise.
- **Multiple inheritance** exists. It is fine for inheriting several *interfaces*, and tricky for inheriting several implementations.
- **Object slicing**: assigning a `Circle` to a plain `Shape` *value* copies only the base part. Polymorphism works only through references and pointers.

::quiz
? What does `override` do?
+ It makes the compiler check that the function really overrides a virtual base function
- It makes the function virtual in the base
- It hides the base function
> It turns silent typos into compile errors.
? Why does a polymorphic base class need a virtual destructor?
+ So deleting through a base pointer runs the derived destructor too
- So the class can be copied
- To make it abstract
> Without it, deleting a derived object through a base pointer is undefined behaviour.
? Your shapes are always Circle or Rect, never anything else. What's the lightweight alternative to a virtual hierarchy?
+ std::variant<Circle, Rect> with std::visit
- void pointers
- macros
> A closed set of alternatives fits variant: no heap, no vtable.
::
