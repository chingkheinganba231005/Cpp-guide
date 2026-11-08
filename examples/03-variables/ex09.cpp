#include <iostream>

int main() {
    double d = 7.9;
    int i = static_cast<int>(d);      // explicit conversion: truncates towards zero
    std::cout << i << '\n';
    int total = 7, count = 2;
    std::cout << total / count << " vs " << static_cast<double>(total) / count << '\n';
}
