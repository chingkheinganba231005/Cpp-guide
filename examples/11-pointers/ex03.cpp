#include <iostream>
#include <string>

struct Player { std::string name; int hp; };

void heal(Player* p) {
    if (p) p->hp += 10;       // p->hp is shorthand for (*p).hp
}

int main() {
    Player hero{"Ada", 50};
    heal(&hero);
    heal(nullptr);            // safe: heal checks for null
    std::cout << hero.name << " has " << hero.hp << " hp\n";
}
