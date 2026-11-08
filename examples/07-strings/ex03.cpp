#include <format>
#include <iostream>

int main() {
    double price = 3.14159;
    int qty = 7;
    std::cout << std::format("{} items at ${:.2f} = ${:.2f}\n", qty, price, qty * price);
    std::cout << std::format("|{:<8}|{:^8}|{:>8}|\n", "left", "mid", "right");
    std::cout << std::format("{:08.3f} {:x} {:b} {:e}\n", price, 255, 5, 123456.0);
}
