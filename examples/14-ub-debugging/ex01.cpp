#include <climits>
#include <iostream>

bool next_is_bigger(int i) { return i + 1 > i; }     // overflow is UB, so the compiler assumes it can't happen

int main() {
    std::cout << std::boolalpha << next_is_bigger(5) << ' ' << next_is_bigger(INT_MAX) << '\n';
}
