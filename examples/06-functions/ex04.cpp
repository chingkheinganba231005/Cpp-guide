#include <iostream>

void try_to_change(int n) {
    n = 100;
}

int main() {
    int x = 1;
    try_to_change(x);
    std::cout << x << '\n';
}
