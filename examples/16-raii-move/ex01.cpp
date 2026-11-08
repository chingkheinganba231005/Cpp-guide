#include <iostream>
#include <string>

struct Tracer {
    std::string name;
    explicit Tracer(std::string n) : name(std::move(n)) { std::cout << "construct " << name << '\n'; }
    ~Tracer() { std::cout << "destroy   " << name << '\n'; }
};

int main() {
    Tracer a{"a"};
    {
        Tracer b{"b"};
        Tracer c{"c"};
        std::cout << "-- leaving inner block --\n";
    }
    Tracer d{"d"};
    std::cout << "-- leaving main --\n";
}
