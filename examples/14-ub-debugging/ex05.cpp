#include <iostream>
#include <string>
#include <string_view>

std::string_view first_word(std::string s) {   // s is a local copy...
    return std::string_view(s).substr(0, s.find(' '));    // ...and we return a view into it
}

int main() {
    std::string_view w = first_word("supercalifragilistic words longer than the small-string buffer");
    std::cout << w << '\n';
}
