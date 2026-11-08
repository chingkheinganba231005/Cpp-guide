#include <iostream>
#include <utility>
#include <vector>

int main() {
    std::vector v{1, 2, 3};          // std::vector<int>
    std::pair p{2.5, 'x'};           // std::pair<double, char>
    std::cout << v.size() << ' ' << p.first << p.second << '\n';
}
