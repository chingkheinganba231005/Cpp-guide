#include <iostream>

int main() {
    char c = 'A';
    std::cout << c << " has code " << static_cast<int>(c) << '\n';
    std::cout << static_cast<char>(c + 2) << '\n';     // 'C'
    std::cout << ('7' - '0') << '\n';                  // turn a digit character into its value: 7
}
