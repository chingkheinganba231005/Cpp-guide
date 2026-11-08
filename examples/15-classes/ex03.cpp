#include <iostream>
#include <stdexcept>
#include <string>

class Account {
public:
    explicit Account(std::string owner, long long cents = 0)    // constructor
        : owner_(std::move(owner)), balance_cents_(cents) {
        if (cents < 0) throw std::invalid_argument("negative opening balance");
    }

    void deposit(long long cents) {
        if (cents <= 0) throw std::invalid_argument("deposit must be positive");
        balance_cents_ += cents;
    }

    bool withdraw(long long cents) {          // refuses rather than breaking the invariant
        if (cents <= 0 || cents > balance_cents_) return false;
        balance_cents_ -= cents;
        return true;
    }

    long long balance() const { return balance_cents_; }
    const std::string& owner() const { return owner_; }

private:
    std::string owner_;
    long long balance_cents_;                 // invariant: never negative
};

int main() {
    Account acc{"Ada", 10'000};
    acc.deposit(2'500);
    std::cout << std::boolalpha << acc.withdraw(50'000) << ' ' << acc.withdraw(5'000) << '\n';
    std::cout << acc.owner() << " has " << acc.balance() / 100.0 << '\n';
    // acc.balance_cents_ = -1;   // compile error: private
}
