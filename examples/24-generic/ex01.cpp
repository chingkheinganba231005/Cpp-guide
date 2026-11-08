#include <concepts>
#include <iostream>
#include <string>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>                                 // T must satisfy Numeric
T square(T x) { return x * x; }

template <typename T>
concept Shape = requires(const T& s) {               // a custom concept: "has an area() returning something numeric"
    { s.area() } -> std::convertible_to<double>;
};

struct Square { double side; double area() const { return side * side; } };

double total_area(const Shape auto& a, const Shape auto& b) { return a.area() + b.area(); }

int main() {
    std::cout << square(7) << ' ' << square(1.5) << '\n';
    std::cout << total_area(Square{2}, Square{3}) << '\n';
}
