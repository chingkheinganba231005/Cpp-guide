#include <iostream>

int main() {
    int n = 5;
    n += 3;       // n = n + 3  -> 8
    n *= 2;       // n = n * 2  -> 16
    ++n;          // add one     -> 17
    int a = n++;  // a gets the OLD value (17), then n becomes 18
    int b = ++n;  // n becomes 19 first, then b gets 19
    std::cout << n << ' ' << a << ' ' << b << '\n';
}
