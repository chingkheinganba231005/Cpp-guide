#include <iostream>
#include <string>
#include <type_traits>

template <typename T>
std::string describe(const T& value) {
    if constexpr (std::is_arithmetic_v<T>) return "number " + std::to_string(value);
    else if constexpr (std::is_same_v<T, std::string>) return "text '" + value + "'";
    else return "something else";
}

struct Opaque {};

int main() {
    std::cout << describe(42) << ", " << describe(2.5) << ", " << describe(std::string("hi")) << ", " << describe(Opaque{}) << '\n';
}
