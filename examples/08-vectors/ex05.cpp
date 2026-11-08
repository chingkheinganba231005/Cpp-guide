#include <array>
#include <iostream>

int main() {
    std::array<double, 3> position{1.0, 2.5, -4.0};     // x, y, z
    double len2 = 0;
    for (double c : position) len2 += c * c;
    std::cout << "size " << position.size() << ", length^2 " << len2 << '\n';
}
