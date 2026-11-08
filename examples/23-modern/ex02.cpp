#include <cstdint>
#include <iostream>

enum class Side : std::uint8_t { Buy, Sell };          // scoped, typed, 1 byte
enum class Color { Red, Green, Blue };

const char* to_string(Side s) { return s == Side::Buy ? "BUY" : "SELL"; }

int main() {
    Side s = Side::Sell;
    std::cout << to_string(s) << ", size " << sizeof(Side) << " byte\n";
    // if (s == Color::Red) {}   // compile error: different enum types can't be mixed
    // int n = s;                // compile error: no silent conversion to int
    std::cout << static_cast<int>(Color::Blue) << '\n';      // explicit when you really need the number
}
