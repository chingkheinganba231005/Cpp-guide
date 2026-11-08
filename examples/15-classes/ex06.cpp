#include <algorithm>
#include <iostream>
#include <string>

class Health {
public:
    explicit Health(int max) : max_(max), current_(max) {}
    void damage(int amount) { current_ = std::max(0, current_ - std::max(0, amount)); }
    void heal(int amount) { current_ = std::min(max_, current_ + std::max(0, amount)); }
    bool alive() const { return current_ > 0; }
    double fraction() const { return static_cast<double>(current_) / max_; }
private:
    int max_, current_;          // invariant: 0 <= current_ <= max_
};

int main() {
    Health h{100};
    h.damage(130);
    std::cout << std::boolalpha << h.alive() << ' ' << h.fraction() << '\n';
    h.heal(40);
    std::cout << h.alive() << ' ' << h.fraction() << '\n';
}
