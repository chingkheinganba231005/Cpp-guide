#include <iostream>

int main() {
    int a = 42;
    double b = 3.5;
    char c = 'z';
    std::cout << "a lives at " << &a << " and takes " << sizeof a << " bytes\n";
    std::cout << "b lives at " << &b << " and takes " << sizeof b << " bytes\n";
    std::cout << "c lives at " << static_cast<void*>(&c) << " and takes " << sizeof c << " byte\n";
}
