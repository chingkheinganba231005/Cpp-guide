#include <iostream>
#include <memory>
#include <numbers>
#include <vector>

class Shape {
public:
    virtual ~Shape() = default;              // essential: see below
    virtual double area() const = 0;         // "= 0": pure virtual. Shape is abstract (an interface)
    virtual const char* name() const { return "shape"; }
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_(r) {}
    double area() const override { return std::numbers::pi * r_ * r_; }
    const char* name() const override { return "circle"; }
private:
    double r_;
};

class Rect : public Shape {
public:
    Rect(double w, double h) : w_(w), h_(h) {}
    double area() const override { return w_ * h_; }
    const char* name() const override { return "rectangle"; }
private:
    double w_, h_;
};

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(1.0));
    shapes.push_back(std::make_unique<Rect>(2.0, 3.0));
    double total = 0;
    for (const auto& s : shapes) {
        std::cout << s->name() << ": " << s->area() << '\n';   // the right area() for each real type
        total += s->area();
    }
    std::cout << "total " << total << '\n';
}
