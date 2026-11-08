#include <cmath>
#include <iostream>
#include <vector>

double max_abs(const std::vector<double>& v) {
    double m = 0;
    for (double x : v) m = std::max(m, std::abs(x));
    return m;
}

void normalise(std::vector<double>& v) {
    double m = max_abs(v);
    if (m == 0) return;
    for (double& x : v) x /= m;
}

int main() {
    std::vector<double> v{2, -8, 4};
    normalise(v);
    for (double x : v) std::cout << x << ' ';
    std::cout << '\n';
}
