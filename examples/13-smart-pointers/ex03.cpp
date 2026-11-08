#include <iostream>
#include <memory>
#include <vector>

struct Enemy {
    int hp;
    explicit Enemy(int h) : hp(h) {}
    ~Enemy() { std::cout << "  enemy with " << hp << " hp destroyed\n"; }
};

std::unique_ptr<Enemy> spawn(int hp) {
    return std::make_unique<Enemy>(hp);          // ownership leaves the function with the return value
}

int main() {
    std::vector<std::unique_ptr<Enemy>> enemies;  // the vector owns the enemies
    auto e = spawn(30);
    enemies.push_back(std::move(e));              // ownership moves into the vector; e is now empty
    enemies.push_back(spawn(50));
    std::cout << std::boolalpha << "e is empty: " << (e == nullptr) << '\n';
    std::cout << enemies.size() << " enemies\n";
    enemies.erase(enemies.begin());               // removing an element destroys that enemy
    std::cout << "clearing the rest\n";
}
