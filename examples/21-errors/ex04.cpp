#include <charconv>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>

enum class ParseError { Empty, NotANumber, OutOfRange };

std::string_view to_string(ParseError e) {
    switch (e) {
        case ParseError::Empty: return "empty input";
        case ParseError::NotANumber: return "not a number";
        case ParseError::OutOfRange: return "out of range";
    }
    return "?";
}

std::expected<int, ParseError> parse_qty(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    int value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);   // fast, no exceptions, no allocation
    if (ec == std::errc::result_out_of_range) return std::unexpected(ParseError::OutOfRange);
    if (ec != std::errc{} || ptr != s.data() + s.size()) return std::unexpected(ParseError::NotANumber);
    return value;
}

int main() {
    for (std::string_view s : {"250", "", "12a", "99999999999"}) {
        auto r = parse_qty(s);
        if (r) std::cout << "'" << s << "' -> " << *r << '\n';
        else   std::cout << "'" << s << "' -> error: " << to_string(r.error()) << '\n';
    }
}
