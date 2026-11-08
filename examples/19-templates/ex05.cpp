#include <iostream>
#include <string>

auto twice(auto x) { return x + x; }         // a function template, in short form

int main() {
    auto add = [](auto a, auto b) { return a + b; };
    std::cout << add(1, 2) << ' ' << add(1.5, 2.25) << ' ' << add(std::string("ab"), std::string("cd")) << '\n';
    std::cout << twice(21) << ' ' << twice(std::string("ha")) << '\n';
}
