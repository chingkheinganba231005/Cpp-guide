#include <iostream>
#include <iomanip>

int main() {
    double a = 0.1 + 0.2;
    std::cout << std::setprecision(17) << a << '\n';
    std::cout << std::boolalpha << (a == 0.3) << '\n';
}
