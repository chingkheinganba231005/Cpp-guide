#include <iostream>
#include <string>

int main() {
    std::string name = "Ada";
    std::string greeting = "Hello, " + name + "!";      // + concatenates
    std::cout << greeting << '\n';
    std::cout << "length: " << greeting.size() << '\n';  // Python: len(greeting)
    std::cout << "first: " << greeting[0] << ", last: " << greeting.back() << '\n';
    greeting[0] = 'J';                                    // strings are mutable
    greeting += " Welcome.";                              // append in place
    std::cout << greeting << '\n';
}
