#include <iostream>

int main() {
    int total = 7384;
    int h = total / 3600, m = (total % 3600) / 60, s = total % 60;
    std::cout << h << "h " << m << "m " << s << "s\n";
}
