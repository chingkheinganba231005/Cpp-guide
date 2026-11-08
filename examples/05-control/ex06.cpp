#include <iostream>
#include <vector>

int main() {
    std::vector<int> primes{2, 3, 5, 7, 11};
    for (int p : primes) {                // Python: for p in primes:
        std::cout << p * p << ' ';
    }
    std::cout << '\n';
}
