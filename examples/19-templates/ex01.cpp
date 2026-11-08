#include <iostream>
#include <string>

template <typename T>
T largest(const T& a, const T& b) {
    return (a < b) ? b : a;
}

int main() {
    std::cout << largest(3, 7) << '\n';                                   // T = int
    std::cout << largest(2.5, 1.5) << '\n';                               // T = double
    std::cout << largest(std::string("pear"), std::string("apple")) << '\n';   // T = std::string
    std::cout << largest<double>(3, 2.5) << '\n';                         // T given explicitly
}
