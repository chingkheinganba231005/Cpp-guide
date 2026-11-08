#include <iostream>

int main() {
    int age = 20;
    bool adult = age >= 18;
    bool teen = age >= 13 && age <= 19;     // && is "and"
    bool weekend = false, holiday = true;
    bool free_day = weekend || holiday;     // || is "or"
    std::cout << std::boolalpha << adult << ' ' << teen << ' ' << free_day << ' ' << !adult << '\n';
}
