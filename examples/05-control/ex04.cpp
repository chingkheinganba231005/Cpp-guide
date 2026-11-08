#include <iostream>

int main() {
    int n = 27, steps = 0;                // the Collatz sequence: does it always reach 1?
    while (n != 1) {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        ++steps;
    }
    std::cout << "27 reaches 1 after " << steps << " steps\n";
}
