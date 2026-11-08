#include <iostream>
#include "autograd.hpp"

double f(double a, double b) { return std::tanh(a * b + a) * b; }

int main() {
    Value a(0.7), b(-1.3);
    Value out = (a * b + a).tanh() * b;
    out.backward();
    double h = 1e-6;
    double da = (f(0.7 + h, -1.3) - f(0.7 - h, -1.3)) / (2 * h);    // central difference
    double db = (f(0.7, -1.3 + h) - f(0.7, -1.3 - h)) / (2 * h);
    std::cout << "f = " << out.data() << '\n';
    std::cout << "df/da autograd " << a.grad() << "  numeric " << da << '\n';
    std::cout << "df/db autograd " << b.grad() << "  numeric " << db << '\n';
}
