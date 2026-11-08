#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    std::vector<int> v{7, 2, 9, 4, 4, 1};
    std::ranges::sort(v);
    std::cout << "sorted: "; for (int x : v) std::cout << x << ' '; std::cout << '\n';
    std::cout << "sum " << std::accumulate(v.begin(), v.end(), 0)
              << ", max " << std::ranges::max(v)
              << ", count of 4: " << std::ranges::count(v, 4) << '\n';
    std::ranges::reverse(v);
    auto it = std::ranges::find(v, 9);
    std::cout << "9 found at index " << (it - v.begin()) << '\n';
}
