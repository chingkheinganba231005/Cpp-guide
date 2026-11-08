#include <iostream>
#include <string>

int main() {
    std::string name;
    int age;
    std::cout << "Name and age? ";
    std::cin >> name >> age;
    std::cout << '\n' << name << " will be " << age + 1 << " next year\n";
}
