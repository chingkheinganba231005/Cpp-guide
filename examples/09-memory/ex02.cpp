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
