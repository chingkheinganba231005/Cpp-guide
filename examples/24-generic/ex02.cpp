#include <concepts>
#include <string>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>
T square(T x) { return x * x; }

int main() {
    square(std::string("hi"));
}
