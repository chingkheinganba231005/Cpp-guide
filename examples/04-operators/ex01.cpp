#include <iostream>

int main() {
    int a = 17, b = 5;
    std::cout << a + b << ' ' << a - b << ' ' << a * b << '\n';
    std::cout << a / b << '\n';     // integer division: 3 (the .4 is thrown away)
    std::cout << a % b << '\n';     // remainder: 2
    std::cout << 17.0 / 5 << '\n';  // 3.4: one double operand makes it floating-point
    std::cout << -7 / 2 << ' ' << -7 % 2 << '\n';  // C++ truncates towards zero
}
