#include <iostream>

int add(int a, int b) { return a + b; }
int mul(int a, int b) { return a * b; }

int main(int argc, char** argv) {
    int (*op)(int, int) = add;          // a pointer to a function taking (int, int) and returning int
    std::cout << op(3, 4) << ' ';
    op = mul;
    std::cout << op(3, 4) << '\n';
    std::cout << "program name: " << argv[0] << ", argc = " << argc << '\n';
}
