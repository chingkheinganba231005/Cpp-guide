#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

[[nodiscard]] int compute() { return 42; }        // warn if the caller ignores the result

struct Config {                                    // designated initialisers (C++20): name the fields
    int width = 800;
    int height = 600;
    bool vsync = true;
};

int main() {
    Config c{.width = 1920, .height = 1080};
    std::cout << c.width << 'x' << c.height << ' ' << std::boolalpha << c.vsync << '\n';
    auto n = compute();
    std::vector<std::string> names{"b", "a"};
    std::ranges::sort(names);
    std::cout << n << ' ' << names[0] << '\n';
    using namespace std::literals;
    auto s = "a std::string literal"s;              // the s suffix makes a std::string
    std::cout << s.size() << '\n';
}
