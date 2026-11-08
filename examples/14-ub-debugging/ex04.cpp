#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{1, 2, 3};
    int& first = v[0];              // a reference into the vector's heap block
    for (int i = 0; i < 100; ++i) v.push_back(i);   // the vector grows: its block moves
    std::cout << first << '\n';     // first refers to the OLD, freed block
}
