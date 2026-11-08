#include <iostream>

template <typename T>
T largest(const T& a, const T& b) { return (a < b) ? b : a; }

struct Point { int x, y; };          // no operator<

int main() {
    largest(Point{1, 2}, Point{3, 4});
}
