#include <cstdint>
#include <iostream>

std::uint64_t factorial(int n) {
    if (n <= 1) return 1;            // base case: stops the recursion
    return n * factorial(n - 1);     // recursive case
}

int main() {
    for (int n : {5, 10, 20}) std::cout << n << "! = " << factorial(n) << '\n';
}
