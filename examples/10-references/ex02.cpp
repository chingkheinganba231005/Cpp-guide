#include <iostream>

void add_bonus(int& points) {     // points is another name for the caller's variable
    points += 100;
}

void swap_values(int& a, int& b) {
    int tmp = a;
    a = b;
    b = tmp;
}

int main() {
    int alice = 50, bob = 70;
    add_bonus(alice);
    swap_values(alice, bob);
    std::cout << "alice " << alice << ", bob " << bob << '\n';
}
