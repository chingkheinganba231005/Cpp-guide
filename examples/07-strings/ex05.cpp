#include <iostream>
#include <string>

int main() {
    std::string full;
    std::getline(std::cin, full);                 // reads "Ada Lovelace"
    int x, sum = 0;
    while (std::cin >> x) sum += x;               // read numbers until input ends
    std::cout << full << ": sum = " << sum << '\n';
}
