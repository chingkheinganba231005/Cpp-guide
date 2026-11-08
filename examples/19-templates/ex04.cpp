#include <iostream>
#include <string>

template <typename T>
std::string describe(const T&) { return "something"; }

template <>
std::string describe<bool>(const bool& b) { return b ? "yes" : "no"; }   // full specialisation for bool

int main() {
    std::cout << describe(42) << ' ' << describe(true) << '\n';
}
