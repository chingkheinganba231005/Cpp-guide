#include <iostream>
#include <vector>

int main() {
    std::vector<int> scores{90, 72, 85};     // Python: scores = [90, 72, 85]
    scores.push_back(64);                    // append
    std::cout << "size " << scores.size() << ", first " << scores[0] << ", last " << scores.back() << '\n';
    scores[1] = 75;                          // change an element
    int total = 0;
    for (int s : scores) total += s;
    std::cout << "average " << static_cast<double>(total) / scores.size() << '\n';
    scores.pop_back();                       // remove the last element
    scores.insert(scores.begin(), 100);      // insert at the front (slow for big vectors)
    for (int s : scores) std::cout << s << ' ';
    std::cout << '\n';
}
