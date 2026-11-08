#include <iostream>
#include <numeric>
#include <stdexcept>

class Fraction {
public:
    Fraction(long long n, long long d) : num_(n), den_(d) {
        if (d == 0) throw std::invalid_argument("zero denominator");
        if (den_ < 0) { num_ = -num_; den_ = -den_; }
        long long g = std::gcd(num_, den_);
        if (g) { num_ /= g; den_ /= g; }
    }
    double value() const { return static_cast<double>(num_) / den_; }
    Fraction times(const Fraction& o) const { return Fraction(num_ * o.num_, den_ * o.den_); }
    long long num() const { return num_; }
    long long den() const { return den_; }
private:
    long long num_, den_;   // invariant: lowest terms, den_ > 0
};

int main() {
    Fraction f(6, -8), g(2, 3);
    Fraction p = f.times(g);
    std::cout << f.num() << '/' << f.den() << " = " << f.value() << ", times 2/3 = " << p.num() << '/' << p.den() << '\n';
}
