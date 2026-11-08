#include <iostream>

int main() {
    int a[5] = {10, 20, 30, 40, 50};
    int* p = a;                     // an array name decays to a pointer to its first element
    std::cout << *p << ' ' << *(p + 1) << ' ' << p[2] << '\n';    // p[i] is exactly *(p + i)
    std::cout << "p     = " << p << '\n' << "p + 1 = " << p + 1 << "  (4 bytes later)\n";
    for (int* q = a; q != a + 5; ++q) std::cout << *q << ' ';    // walk with a pointer
    std::cout << '\n';
}
