#include <cctype>
#include <iostream>
#include <string>

int main() {
    std::string s = "The quick brown fox";
    std::cout << s.substr(4, 5) << '\n';                       // "quick"
    std::size_t pos = s.find("brown");
    std::cout << "brown at index " << pos << '\n';
    std::cout << std::boolalpha << s.starts_with("The") << ' ' << s.contains("dog") << '\n';
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));   // in-place upper case
    std::cout << s << '\n';
    int n = std::stoi("123") + 1;
    std::cout << n << ' ' << std::to_string(3.5) << '\n';
}
