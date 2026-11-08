#include <iostream>

int main() {
    for (int i = 1; i <= 20; ++i) {
        if (i % 15 == 0) std::cout << "FizzBuzz ";
        else if (i % 3 == 0) std::cout << "Fizz ";
        else if (i % 5 == 0) std::cout << "Buzz ";
        else std::cout << i << ' ';
    }
    std::cout << "\n\n";
    for (int r = 1; r <= 5; ++r) {
        for (int c = 1; c <= 5; ++c) std::cout << r * c << '\t';
        std::cout << '\n';
    }
}
