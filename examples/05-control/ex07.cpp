#include <iostream>

int main() {
    for (int n = 2; n < 50; ++n) {
        bool prime = true;
        for (int d = 2; d * d <= n; ++d) {
            if (n % d == 0) { prime = false; break; }   // leave the inner loop early
        }
        if (!prime) continue;                            // skip to the next n
        std::cout << n << ' ';
    }
    std::cout << '\n';
}
