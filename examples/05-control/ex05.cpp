#include <iostream>

int main() {
    for (int i = 0; i < 5; ++i) {          // Python: for i in range(5):
        std::cout << i << ' ';
    }
    std::cout << '\n';
    for (int i = 10; i > 0; i -= 3) std::cout << i << ' ';   // range(10, 0, -3)
    std::cout << '\n';
}
