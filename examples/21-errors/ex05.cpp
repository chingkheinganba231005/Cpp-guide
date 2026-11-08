#include <cassert>
#include <vector>

double average(const std::vector<double>& v) {
    assert(!v.empty() && "average of an empty vector");   // a precondition
    double s = 0; for (double x : v) s += x;
    return s / v.size();
}

int main() {
    std::vector<double> none;
    return static_cast<int>(average(none));
}
