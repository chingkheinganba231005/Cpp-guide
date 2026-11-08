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
