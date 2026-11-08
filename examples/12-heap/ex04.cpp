#include <iostream>
#include <vector>

int main() {
    int n = 1'000'000;
    std::vector<double> data(n, 1.5);   // a heap allocation, owned by the vector
    std::cout << data.size() << " doubles on the heap at " << data.data() << '\n';
}   // the vector's destructor frees the memory here, automatically. No leak is possible.
