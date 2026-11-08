#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{1, 2, 3};
    std::cout << v.at(1) << '\n';
    std::cout << v.at(10) << '\n';     // throws std::out_of_range: the program stops with a message
}
