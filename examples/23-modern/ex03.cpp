#include <iostream>
#include <map>
#include <string>

int main() {
    std::map<std::string, double> prices{{"BTC", 64000.0}, {"ETH", 3100.0}};
    if (auto it = prices.find("ETH"); it != prices.end())   // `it` exists only inside this if/else
        std::cout << "ETH " << it->second << '\n';
    else
        std::cout << "not found\n";
}
