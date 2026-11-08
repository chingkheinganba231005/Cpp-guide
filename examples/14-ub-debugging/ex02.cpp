#include <climits>
#include <iostream>

int main() {
    int big = INT_MAX;
    int volatile one = 1;
    int r = big + one;          // signed overflow
    std::cout << r << '\n';
}
