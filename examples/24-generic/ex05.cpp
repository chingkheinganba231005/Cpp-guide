#include <iostream>

template <typename Derived>
class Shape {                                 // the base knows its derived type
public:
    double area() const { return static_cast<const Derived&>(*this).area_impl(); }
    void describe() const { std::cout << Derived::name << " with area " << area() << '\n'; }
};

class Circle : public Shape<Circle> {
public:
    static constexpr const char* name = "circle";
    explicit Circle(double r) : r_(r) {}
    double area_impl() const { return 3.14159265 * r_ * r_; }
private:
    double r_;
};

class Square : public Shape<Square> {
public:
    static constexpr const char* name = "square";
    explicit Square(double s) : s_(s) {}
    double area_impl() const { return s_ * s_; }
private:
    double s_;
};

template <typename S>
void report(const Shape<S>& s) { s.describe(); }   // works for any Shape, with no virtual call

int main() {
    report(Circle{1});
    report(Square{3});
}
