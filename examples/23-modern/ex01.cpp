#include <iostream>
#include <map>
#include <string>
#include <tuple>

struct Quote { double bid, ask; };

std::tuple<int, std::string, double> load() { return {7, "AAPL", 189.5}; }

int main() {
    auto [id, symbol, price] = load();                       // like Python: id, symbol, price = load()
    auto [bid, ask] = Quote{99.5, 100.5};
    std::cout << id << ' ' << symbol << ' ' << price << ", spread " << ask - bid << '\n';
    std::map<std::string, int> stock{{"apples", 3}, {"pears", 5}};
    for (const auto& [fruit, n] : stock) std::cout << fruit << ": " << n << '\n';
}
