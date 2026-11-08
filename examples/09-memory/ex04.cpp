#include <iostream>

struct Bad  { char a; double b; char c; };   // 1 + (7 padding) + 8 + 1 + (7 padding)
struct Good { double b; char a; char c; };   // 8 + 1 + 1 + (6 padding)

int main() {
    std::cout << sizeof(Bad) << ' ' << sizeof(Good) << '\n';
}
