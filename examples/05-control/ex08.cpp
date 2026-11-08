#include <iostream>

int main() {
    for (int i = 0; i < 3; ++i) {
        int square = i * i;
    }
    std::cout << square << '\n';   // error: square only existed inside the loop
}
