#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string line, word, longest;
    std::getline(std::cin, line);
    std::istringstream in(line);
    int count = 0;
    while (in >> word) {
        ++count;
        if (word.size() > longest.size()) longest = word;
    }
    std::cout << count << " words, longest: " << longest << '\n';
}
