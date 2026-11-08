#include <iostream>

#include "geometry.hpp"

int main() {
    std::vector<geo::Point> route{{0, 0}, {3, 4}, {3, 10}};
    std::cout << "route length: " << geo::path_length(route) << '\n';
}
