#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct Trade { std::string symbol; double price; int qty; };

int main() {
    std::vector<Trade> trades{{"AAPL", 189.5, 100}, {"MSFT", 410.2, 50}, {"AAPL", 190.1, 200}, {"NVDA", 120.7, 300}};

    std::ranges::sort(trades, {}, &Trade::price);                        // sort by price (a "projection")
    for (const auto& t : trades) std::cout << t.symbol << '@' << t.price << ' ';
    std::cout << '\n';

    auto big = std::ranges::count_if(trades, [](const Trade& t) { return t.qty >= 200; });
    double notional = std::accumulate(trades.begin(), trades.end(), 0.0,
                                      [](double acc, const Trade& t) { return acc + t.price * t.qty; });
    auto most = std::ranges::max_element(trades, {}, &Trade::qty);
    std::cout << big << " big trades, notional " << notional << ", largest qty " << most->symbol << '\n';

    std::erase_if(trades, [](const Trade& t) { return t.symbol == "AAPL"; });    // remove matching elements (C++20)
    std::cout << trades.size() << " left\n";

    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 1);                                        // 1..10
    std::vector<int> squares;
    std::ranges::transform(v, std::back_inserter(squares), [](int x) { return x * x; });   // Python: [x*x for x in v]
    std::cout << "binary search for 49: " << std::boolalpha << std::ranges::binary_search(squares, 49) << '\n';
}
