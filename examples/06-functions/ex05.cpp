#include <iostream>
#include <string>

int twice(int x) { return 2 * x; }
double twice(double x) { return 2 * x; }
std::string twice(const std::string& s) { return s + s; }

int main() {
    std::cout << twice(21) << ' ' << twice(1.25) << ' ' << twice(std::string{"ab"}) << '\n';
}
