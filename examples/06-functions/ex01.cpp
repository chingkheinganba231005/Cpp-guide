#include <iostream>
#include <string>

double area_of_circle(double radius) {     // return type, name, (parameter type and name)
    const double pi = 3.141592653589793;
    return pi * radius * radius;
}

void greet(const std::string& name) {      // void: returns nothing (chapter 9 explains const&)
    std::cout << "Hello, " << name << "!\n";
}

int main() {
    std::cout << area_of_circle(2.0) << '\n';
    greet("Ada");
}
