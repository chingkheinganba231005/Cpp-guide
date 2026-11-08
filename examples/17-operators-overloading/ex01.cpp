#include <cmath>
#include <iostream>

struct Vec2 {
    double x = 0, y = 0;

    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }   // compound assignment modifies *this
    Vec2& operator*=(double k) { x *= k; y *= k; return *this; }
    double length() const { return std::hypot(x, y); }
};

// Binary operators as free functions, built on the compound ones
inline Vec2 operator+(Vec2 a, const Vec2& b) { return a += b; }
inline Vec2 operator-(const Vec2& a, const Vec2& b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(Vec2 v, double k) { return v *= k; }
inline Vec2 operator*(double k, Vec2 v) { return v *= k; }        // so 2 * v works too
inline Vec2 operator-(const Vec2& v) { return {-v.x, -v.y}; }     // unary minus
inline double dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }

std::ostream& operator<<(std::ostream& os, const Vec2& v) {      // printing
    return os << '(' << v.x << ", " << v.y << ')';
}

int main() {
    Vec2 pos{1, 2}, vel{0.5, -1}, gravity{0, -9.81};
    double dt = 0.1;
    vel += gravity * dt;              // reads like physics
    pos += vel * dt;
    std::cout << "pos " << pos << ", vel " << vel << ", speed " << vel.length() << '\n';
    std::cout << "dot " << dot(Vec2{1, 0}, Vec2{0, 1}) << ", -pos " << -pos << ", 2*pos " << 2 * pos << '\n';
}
