#include <iostream>
#include <string>
#include <string_view>

int count_vowels(std::string_view s) {
    int n = 0;
    for (char c : s) n += (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
    return n;
}

int main() {
    std::string owned = "programming";
    std::cout << count_vowels(owned) << ' ' << count_vowels("literal works too") << '\n';
}
