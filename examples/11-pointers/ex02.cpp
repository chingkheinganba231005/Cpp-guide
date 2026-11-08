#include <iostream>

int main() {
    int* p = nullptr;
    std::cout << "about to dereference a null pointer..." << std::endl;
    std::cout << *p << '\n';
}
