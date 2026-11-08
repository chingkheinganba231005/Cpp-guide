So far you have used types other people wrote: `int`, `std::string`, `std::vector`. Now you will write your own. A **class** bundles data with the functions that work on it, and protects the rules that keep that data valid. Every engine, library and trading system is built out of well-designed classes.

## struct: a bundle of data

```cpp run
#include <cmath>
#include <iostream>

struct Vec2 {
    double x = 0;      // members, with default values
    double y = 0;

    double length() const {             // a member function ("method")
        return std::sqrt(x * x + y * y);
    }
    void scale(double k) { x *= k; y *= k; }
};

int main() {
    Vec2 v{3, 4};                   // aggregate initialisation, in member order
    std::cout << v.length() << '\n';
    v.scale(2);
    std::cout << v.x << ", " << v.y << " (length " << v.length() << ")\n";
    Vec2 origin;                    // uses the defaults: (0, 0)
    std::cout << origin.length() << '\n';
}
```

Inside a member function, members are used directly (`x`, not `self.x`). The hidden pointer to the current object is called `this`, the equivalent of Python's `self`, but you rarely need to write it.

`const` after the parameter list (`length() const`) promises the method doesn't modify the object. Mark every method that only reads as `const`. Otherwise you can't call it on a `const` object or through a `const&`.

:::python
```python
class Vec2:
    def __init__(self, x=0.0, y=0.0):
        self.x, self.y = x, y
    def length(self):
        return (self.x**2 + self.y**2) ** 0.5
```
A Python object is a dictionary-like bag you can add attributes to at any time. A C++ object has exactly the members declared, at fixed offsets: `sizeof(Vec2)` is 16 bytes, and nothing else.
:::

## class: data with rules (invariants)

Often some combinations of values are invalid: a bank balance that must never go negative, a fraction whose denominator must not be zero, an order book whose prices must stay sorted. Such a rule is an **invariant**. To protect an invariant, make the data **private** and allow changes only through **public** member functions that check the rule:

```cpp run
#include <iostream>
#include <stdexcept>
#include <string>

class Account {
public:
    explicit Account(std::string owner, long long cents = 0)    // constructor
        : owner_(std::move(owner)), balance_cents_(cents) {
        if (cents < 0) throw std::invalid_argument("negative opening balance");
    }

    void deposit(long long cents) {
        if (cents <= 0) throw std::invalid_argument("deposit must be positive");
        balance_cents_ += cents;
    }

    bool withdraw(long long cents) {          // refuses rather than breaking the invariant
        if (cents <= 0 || cents > balance_cents_) return false;
        balance_cents_ -= cents;
        return true;
    }

    long long balance() const { return balance_cents_; }
    const std::string& owner() const { return owner_; }

private:
    std::string owner_;
    long long balance_cents_;                 // invariant: never negative
};

int main() {
    Account acc{"Ada", 10'000};
    acc.deposit(2'500);
    std::cout << std::boolalpha << acc.withdraw(50'000) << ' ' << acc.withdraw(5'000) << '\n';
    std::cout << acc.owner() << " has " << acc.balance() / 100.0 << '\n';
    // acc.balance_cents_ = -1;   // compile error: private
}
```

The only difference between `struct` and `class` is the default access: `struct` members are public by default, `class` members private. A common convention: `struct` for plain bundles of data with no invariants, `class` when there are rules to protect. The trailing underscore in `owner_` is a common naming style for private members.

## Constructors

A **constructor** has the class's name and no return type. It runs when an object is created and must leave the object valid.

```cpp norun
class Account {
public:
    explicit Account(std::string owner, long long cents = 0)
        : owner_(std::move(owner)), balance_cents_(cents)    // member initialiser list
    { /* body: extra checks */ }
};
```

- The **member initialiser list** (after the `:`) initialises members directly. Always prefer it to assigning in the body. For `const` and reference members it is the only way.
- Members are initialised **in the order they are declared** in the class, not the order you list them.
- `explicit` stops unwanted implicit conversions (without it, `Account a = "Ada";` would compile). Mark single-argument constructors `explicit` by default.
- `std::move(owner)` hands the string's buffer over instead of copying it (chapter 15).

If you write no constructor, the compiler gives you a default one. If you write any constructor, the default one disappears, unless you ask for it: `Account() = default;`.

## Static members and member functions

`static` members belong to the class itself, not to each object, like Python class attributes:

```cpp run
#include <iostream>

class Entity {
public:
    Entity() : id_(next_id_++) {}
    int id() const { return id_; }
    static int created() { return next_id_; }
private:
    int id_;
    static inline int next_id_ = 0;   // one shared counter for all Entity objects
};

int main() {
    Entity a, b, c;
    std::cout << a.id() << ' ' << b.id() << ' ' << c.id() << ", created " << Entity::created() << '\n';
}
```

## Designing a class: a small game example

```cpp run
#include <algorithm>
#include <iostream>
#include <string>

class Health {
public:
    explicit Health(int max) : max_(max), current_(max) {}
    void damage(int amount) { current_ = std::max(0, current_ - std::max(0, amount)); }
    void heal(int amount) { current_ = std::min(max_, current_ + std::max(0, amount)); }
    bool alive() const { return current_ > 0; }
    double fraction() const { return static_cast<double>(current_) / max_; }
private:
    int max_, current_;          // invariant: 0 <= current_ <= max_
};

int main() {
    Health h{100};
    h.damage(130);
    std::cout << std::boolalpha << h.alive() << ' ' << h.fraction() << '\n';
    h.heal(40);
    std::cout << h.alive() << ' ' << h.fraction() << '\n';
}
```

Notice what the design buys you: **no code outside `Health` can ever create a negative or over-full health value.** Bugs of that kind become impossible, instead of something you check for everywhere.

## Good class design in one list

1. **Make invalid states unrepresentable.** Private data, public operations that keep the invariant.
2. **Small interfaces.** A few clear operations beat a hundred getters and setters.
3. **const-correctness.** Methods that don't modify are `const`; parameters you only read are `const&`.
4. **Initialise everything** in constructors (or with default member values).
5. **Prefer plain structs** when there are no invariants, such as a `Vec2` or a `Pixel`. Not everything needs private data.
6. **Prefer composition** (a class *has* a `Health`) to inheritance (chapter 17).

:::exercise A Fraction class
Write `class Fraction` with a numerator and denominator that is always stored in lowest terms with a positive denominator (use `std::gcd` from `<numeric>`). The constructor throws if the denominator is 0. Add `double value() const` and `Fraction times(const Fraction&) const`. Print `Fraction(6, -8)`.
---solution---
```cpp run
#include <iostream>
#include <numeric>
#include <stdexcept>

class Fraction {
public:
    Fraction(long long n, long long d) : num_(n), den_(d) {
        if (d == 0) throw std::invalid_argument("zero denominator");
        if (den_ < 0) { num_ = -num_; den_ = -den_; }
        long long g = std::gcd(num_, den_);
        if (g) { num_ /= g; den_ /= g; }
    }
    double value() const { return static_cast<double>(num_) / den_; }
    Fraction times(const Fraction& o) const { return Fraction(num_ * o.num_, den_ * o.den_); }
    long long num() const { return num_; }
    long long den() const { return den_; }
private:
    long long num_, den_;   // invariant: lowest terms, den_ > 0
};

int main() {
    Fraction f(6, -8), g(2, 3);
    Fraction p = f.times(g);
    std::cout << f.num() << '/' << f.den() << " = " << f.value() << ", times 2/3 = " << p.num() << '/' << p.den() << '\n';
}
```
:::

::quiz
? What is the difference between struct and class in C++?
+ Only the default access: public for struct, private for class
- Structs can't have member functions
- Classes live on the heap
> Everything else is identical.
? Why mark a method `const`?
+ It promises not to modify the object, so it can be called on const objects and through const references
- It makes the method faster
- It makes the return value constant
> const-correctness is checked by the compiler.
? Why make data members private?
+ So that only the class's own functions can change them, keeping its invariants true
- To save memory
- Because public members are slower
> Encapsulation makes invalid states impossible to create from outside.
::
