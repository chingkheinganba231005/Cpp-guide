#include <iostream>
#include <string>

template <typename... Args>
auto sum(Args... args) { return (args + ...); }              // ((a + b) + c) + ...

template <typename... Args>
void print_all(const Args&... args) {
    ((std::cout << args << ' '), ...);                       // fold over the comma operator
    std::cout << '\n';
}

template <typename T, typename... Rest>
bool all_equal(const T& first, const Rest&... rest) { return ((first == rest) && ...); }

int main() {
    std::cout << sum(1, 2, 3, 4) << ' ' << sum(1.5, 2.5) << '\n';
    print_all("id", 7, 3.14, std::string("ok"), 'x');
    std::cout << std::boolalpha << all_equal(3, 3, 3) << ' ' << all_equal(3, 3, 4) << '\n';
    std::cout << "sizeof...(Args) for 4 args: " << [](auto... a) { return sizeof...(a); }(1, 2, 3, 4) << '\n';
}
