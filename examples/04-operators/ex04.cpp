#include <iostream>

int main() {
    int x = 50;
    std::cout << std::boolalpha << (0 < x < 10) << '\n';        // true! (bug)
    std::cout << (0 < x && x < 10) << '\n';                     // false (correct)
}
