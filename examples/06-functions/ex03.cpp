#include <iostream>

int square(int x);          // declaration: "this exists, defined later"

int main() {
    std::cout << square(9) << '\n';
}

int square(int x) {         // definition
    return x * x;
}
