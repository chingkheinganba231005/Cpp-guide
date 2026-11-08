#include <compare>
#include <format>
#include <iostream>

struct Money {
    long long cents = 0;
    auto operator<=>(const Money&) const = default;
    bool operator==(const Money&) const = default;
};
Money operator+(Money a, Money b) { return {a.cents + b.cents}; }
Money operator-(Money a, Money b) { return {a.cents - b.cents}; }
Money operator*(Money a, long long k) { return {a.cents * k}; }
std::ostream& operator<<(std::ostream& os, Money m) {
    long long a = m.cents < 0 ? -m.cents : m.cents;
    return os << (m.cents < 0 ? "-" : "") << '$' << a / 100 << '.' << std::format("{:02}", a % 100);
}

int main() {
    Money price{1999}, tax{160};
    Money total = (price + tax) * 3;
    std::cout << total << ' ' << std::boolalpha << (price > tax) << '\n';
}
