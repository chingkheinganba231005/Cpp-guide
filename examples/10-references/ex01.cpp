#include <iostream>

int main() {
    int score = 10;
    int& alias = score;      // alias IS score: another name for the same 4 bytes
    alias += 5;
    std::cout << score << '\n';                                  // 15
    std::cout << std::boolalpha << (&alias == &score) << '\n';   // same address
}
