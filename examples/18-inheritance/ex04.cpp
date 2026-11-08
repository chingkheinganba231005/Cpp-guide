#include <iostream>

struct Plain { double x; void f() {} };
struct Poly  { double x; virtual void f() {} virtual ~Poly() = default; };

int main() {
    std::cout << "sizeof(Plain) = " << sizeof(Plain) << ", sizeof(Poly) = " << sizeof(Poly) << " (8 extra bytes: the vtable pointer)\n";
}
