#include <iostream>
#include <ranges>
#include <vector>

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto pipeline = v | std::views::filter([](int x) { return x % 2 == 0; })
                      | std::views::transform([](int x) { return x * x; })
                      | std::views::take(3);
    for (int x : pipeline) std::cout << x << ' ';                 // computed only as you iterate
    std::cout << '\n';
    for (int i : std::views::iota(0, 5) | std::views::reverse) std::cout << i << ' ';   // like reversed(range(5))
    std::cout << '\n';
    for (auto [i, x] : std::views::enumerate(std::vector<char>{'a', 'b', 'c'})) std::cout << i << x << ' ';   // C++23
    std::cout << '\n';
}
