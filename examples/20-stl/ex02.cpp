#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{10, 20, 30};
    for (auto it = v.begin(); it != v.end(); ++it) std::cout << *it << ' ';   // what range-for does for you
    std::cout << '\n' << "distance end - begin = " << (v.end() - v.begin()) << '\n';
}
