#include <cstdint>
#include <iostream>

int main() {
    std::cout << "bool " << sizeof(bool) << ", char " << sizeof(char) << ", int " << sizeof(int)
              << ", long long " << sizeof(long long) << ", float " << sizeof(float)
              << ", double " << sizeof(double) << ", int64_t " << sizeof(std::int64_t) << '\n';
}
