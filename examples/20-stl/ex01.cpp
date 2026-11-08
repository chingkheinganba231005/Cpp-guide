#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

int main() {
    std::string text = "the cat and the dog and the bird";
    std::unordered_map<std::string, int> counts;      // a hash map: Python dict
    std::istringstream in(text);
    for (std::string w; in >> w;) ++counts[w];        // [] creates the entry (with 0) if missing
    std::cout << "the: " << counts["the"] << ", and: " << counts["and"] << '\n';

    std::map<std::string, int> sorted(counts.begin(), counts.end());   // a tree map: keys come out sorted
    for (const auto& [word, n] : sorted) std::cout << word << '=' << n << ' ';
    std::cout << '\n';

    if (auto it = sorted.find("cat"); it != sorted.end()) std::cout << "found cat: " << it->second << '\n';
    std::cout << std::boolalpha << sorted.contains("fish") << '\n';   // C++20
}
