#include <iostream>
#include <string>

int main() {
    auto n = 42;          // int
    auto x = 2.5;         // double
    auto big = 42LL;      // long long (the LL suffix)
    auto s = std::string{"hi"};
    std::cout << n << ' ' << x << ' ' << big << ' ' << s << '\n';
}
