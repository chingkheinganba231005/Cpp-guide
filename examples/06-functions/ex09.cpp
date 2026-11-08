#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{5, -3, 8, -1, 2};
    auto by_abs = [](int a, int b) { return std::abs(a) < std::abs(b); };
    std::sort(v.begin(), v.end(), by_abs);         // Python: sorted(v, key=abs)
    for (int x : v) std::cout << x << ' ';
    std::cout << '\n';

    int threshold = 2;
    auto count = std::count_if(v.begin(), v.end(), [threshold](int x) { return x > threshold; });
    std::cout << count << " values above " << threshold << '\n';
}
