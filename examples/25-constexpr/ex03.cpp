#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

constexpr std::array<std::uint32_t, 256> make_crc_table() {      // CRC-32, used in zip files and network protocols
    std::array<std::uint32_t, 256> t{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t c = i;
        for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        t[i] = c;
    }
    return t;
}
constexpr auto CRC_TABLE = make_crc_table();                     // built by the compiler, stored in the binary

constexpr std::uint32_t crc32(const char* s, std::size_t n) {
    std::uint32_t c = 0xFFFFFFFFu;
    for (std::size_t i = 0; i < n; ++i) c = CRC_TABLE[(c ^ static_cast<std::uint8_t>(s[i])) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

int main() {
    static_assert(crc32("123456789", 9) == 0xCBF43926u);         // the standard CRC-32 check value, verified at compile time
    std::cout << std::hex << crc32("hello", 5) << '\n';
}
