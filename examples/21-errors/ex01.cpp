#include <iostream>
#include <stdexcept>
#include <string>

double parse_price(const std::string& s) {
    std::size_t used = 0;
    double v = std::stod(s, &used);                 // throws std::invalid_argument if s isn't a number
    if (used != s.size()) throw std::invalid_argument("trailing characters in '" + s + "'");
    if (v <= 0) throw std::out_of_range("price must be positive");
    return v;
}

int main() {
    for (std::string s : {"101.25", "abc", "12x", "-5"}) {
        try {
            double price = parse_price(s);          // if this throws, the next line never runs
            std::cout << "parsed " << price << '\n';
        } catch (const std::invalid_argument& e) {
            std::cout << "invalid: " << e.what() << '\n';
        } catch (const std::exception& e) {         // the base of all standard exceptions
            std::cout << "error: " << e.what() << '\n';
        }
    }
}
