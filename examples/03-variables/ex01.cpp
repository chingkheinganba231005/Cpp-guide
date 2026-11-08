#include <iostream>

int main() {
    int age = 21;             // a whole number
    double height = 1.75;     // a number with a fractional part
    char grade = 'A';         // a single character
    bool likes_cpp = true;    // true or false

    std::cout << age << ' ' << height << ' ' << grade << ' ' << likes_cpp << '\n';
    age = age + 1;            // change it later: the type stays int
    std::cout << "next year: " << age << '\n';
}
