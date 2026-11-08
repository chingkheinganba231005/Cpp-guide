#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
void copy_items(T* dst, const T* src, std::size_t n) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(dst, src, n * sizeof(T));         // plain bytes: one fast bulk copy
        std::cout << "memcpy path\n";
    } else {
        for (std::size_t i = 0; i < n; ++i) dst[i] = src[i];   // run each element's copy assignment
        std::cout << "element-wise path\n";
    }
}

int main() {
    int a[3] = {1, 2, 3}, b[3];
    copy_items(b, a, 3);
    std::string s[2] = {"x", "y"}, t[2];
    copy_items(t, s, 2);
    std::cout << std::boolalpha << std::is_integral_v<long> << ' ' << std::is_same_v<int, std::int32_t>
              << ' ' << std::is_pointer_v<int*> << '\n';
}
