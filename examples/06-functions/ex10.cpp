#include <iostream>

int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }

int main() { std::cout << gcd(1071, 462) << '\n'; }
