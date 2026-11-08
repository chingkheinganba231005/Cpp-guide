#include <vector>

int main() {
    std::vector<int> v(4);
    int* p = v.data();
    p[4] = 1;                   // one past the end
    return p[0];
}
