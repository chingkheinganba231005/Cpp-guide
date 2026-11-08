#include <bitset>
#include <iostream>

int main() {
    unsigned a = 0b1100, b = 0b1010;
    std::cout << std::bitset<4>(a & b) << " AND\n"
              << std::bitset<4>(a | b) << " OR\n"
              << std::bitset<4>(a ^ b) << " XOR\n"
              << std::bitset<8>(a << 2) << " shift left by 2 (times 4)\n"
              << std::bitset<4>(a >> 1) << " shift right by 1 (divide by 2)\n";
}
