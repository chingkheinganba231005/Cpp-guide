#include <iostream>

int main() {
    double c = 25;
    std::cout << c << " C = " << c * 9.0 / 5 + 32 << " F\n";
    std::cout << c << " C with integer 9/5: " << c * (9 / 5) + 32 << " F (wrong: 9/5 is 1)\n";
}
