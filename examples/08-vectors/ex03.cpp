#include <iostream>
#include <vector>

int main() {
    std::vector<int> empty;
    std::cout << "empty.size() - 1 = " << empty.size() - 1 << '\n';     // not -1!
    // so `for (std::size_t i = 0; i < v.size() - 1; ++i)` runs (almost) forever on an empty vector
    std::cout << "std::ssize(empty) - 1 = " << std::ssize(empty) - 1 << '\n';   // C++20: a signed size
}
