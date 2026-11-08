#include <array>
#include <iostream>

constexpr long long factorial(int n) {         // may run at compile time or at run time
    long long r = 1;
    for (int i = 2; i <= n; ++i) r *= i;
    return r;
}

int main() {
    constexpr long long f10 = factorial(10);   // computed by the compiler: the program contains the number 3628800
    static_assert(factorial(5) == 120);        // checked while compiling; a wrong value is a compile error
    std::array<int, factorial(4)> slots{};     // usable where a compile-time constant is required
    int n = 12;
    std::cout << f10 << ' ' << slots.size() << ' ' << factorial(n) << " (this one at run time)\n";
}
