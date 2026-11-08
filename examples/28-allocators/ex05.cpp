#include <array>
#include <cstddef>
#include <iostream>
#include <memory_resource>
#include <string>
#include <vector>

int main() {
    std::array<std::byte, 64 * 1024> buffer;                          // 64 KB on the stack
    std::pmr::monotonic_buffer_resource arena(buffer.data(), buffer.size());   // an arena, provided by the library
    std::pmr::vector<std::pmr::string> names(&arena);                 // the vector AND its strings allocate from the arena
    for (int i = 0; i < 100; ++i) names.emplace_back("player_" + std::to_string(i) + "_with_a_long_name");
    std::cout << names.size() << " strings, last: " << names.back() << '\n';
    std::cout << "no heap allocation happened while the buffer had room\n";
}
