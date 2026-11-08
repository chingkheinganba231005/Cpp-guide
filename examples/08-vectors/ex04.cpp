#include <iostream>
#include <vector>

int main() {
    std::vector<int> v;
    std::size_t last_cap = 0;
    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
        if (v.capacity() != last_cap) {
            last_cap = v.capacity();
            std::cout << "size " << v.size() << " -> capacity " << last_cap << '\n';
        }
    }
}
