#include <iostream>
#include <optional>
#include <string>
#include <vector>

struct User { int id; std::string name; };

std::optional<User> find_user(const std::vector<User>& users, int id) {
    for (const auto& u : users) if (u.id == id) return u;
    return std::nullopt;                             // "nothing", like returning None
}

int main() {
    std::vector<User> users{{1, "ada"}, {2, "linus"}};
    if (auto u = find_user(users, 2)) std::cout << "found " << u->name << '\n';
    auto missing = find_user(users, 9);
    std::cout << "user 9: " << (missing ? missing->name : "not found") << '\n';
    std::cout << find_user(users, 9).value_or(User{0, "guest"}).name << '\n';
}
