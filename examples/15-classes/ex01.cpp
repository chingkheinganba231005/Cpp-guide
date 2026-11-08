#include <cmath>
#include <iostream>

struct Vec2 {
    double x = 0;      // members, with default values
    double y = 0;

    double length() const {             // a member function ("method")
        return std::sqrt(x * x + y * y);
    }
    void scale(double k) { x *= k; y *= k; }
};

int main() {
    Vec2 v{3, 4};                   // aggregate initialisation, in member order
    std::cout << v.length() << '\n';
    v.scale(2);
    std::cout << v.x << ", " << v.y << " (length " << v.length() << ")\n";
    Vec2 origin;                    // uses the defaults: (0, 0)
    std::cout << origin.length() << '\n';
}
