#include <iostream>
#include <vector>

const int* find_max(const int* first, const int* last) {
    if (first == last) return nullptr;
    const int* best = first;
    for (const int* p = first + 1; p != last; ++p)
        if (*p > *best) best = p;
    return best;
}

int main() {
    int a[] = {3, 9, 2, 7};
    std::vector<int> v{-5, -1, -9};
    const int* m1 = find_max(a, a + 4);
    const int* m2 = find_max(v.data(), v.data() + v.size());
    std::cout << *m1 << " at index " << (m1 - a) << ", " << *m2 << '\n';
    std::cout << std::boolalpha << (find_max(a, a) == nullptr) << '\n';
}
