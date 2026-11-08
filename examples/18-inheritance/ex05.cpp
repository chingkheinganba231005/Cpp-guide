#include <iostream>
#include <numbers>
#include <variant>
#include <vector>

struct Circle { double r; };
struct Rect { double w, h; };
using Shape = std::variant<Circle, Rect>;

double area(const Circle& c) { return std::numbers::pi * c.r * c.r; }
double area(const Rect& r) { return r.w * r.h; }

int main() {
    std::vector<Shape> shapes{Circle{1.0}, Rect{2.0, 3.0}};      // stored by value, contiguously
    double total = 0;
    for (const auto& s : shapes) total += std::visit([](const auto& x) { return area(x); }, s);
    std::cout << "total " << total << '\n';
}
