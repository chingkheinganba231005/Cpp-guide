#include <iostream>

int main() {
    int score = 72;
    if (score >= 90) {
        std::cout << "A\n";
    } else if (score >= 70) {
        std::cout << "B\n";
    } else {
        std::cout << "C or below\n";
    }
}
