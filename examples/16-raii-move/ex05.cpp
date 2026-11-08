#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string big(1000, 'x');
    std::vector<std::string> v;
    v.push_back(big);               // copies: big is an lvalue, still needed
    v.push_back(std::move(big));    // moves: big's buffer is handed over
    std::cout << "big is now " << (big.empty() ? "empty" : "not empty") << " (valid but unspecified: only reassign or destroy it)\n";
    v.push_back(std::string(500, 'y'));   // a temporary: moved automatically
    std::cout << v.size() << " strings, sizes " << v[0].size() << ' ' << v[1].size() << ' ' << v[2].size() << '\n';
}
