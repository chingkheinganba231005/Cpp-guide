#include <iostream>
#include <utility>

std::pair<int, int> divide(int a, int b) {
    return {a / b, a % b};
}

int main() {
    auto [quotient, remainder] = divide(17, 5);    // Python: q, r = divmod(17, 5)
    std::cout << quotient << " r " << remainder << '\n';
}
