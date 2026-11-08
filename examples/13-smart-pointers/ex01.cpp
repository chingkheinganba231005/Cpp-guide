#include <iostream>
#include <memory>
#include <string>

struct Texture {
    std::string name;
    explicit Texture(std::string n) : name(std::move(n)) { std::cout << "  load " << name << '\n'; }
    ~Texture() { std::cout << "  free " << name << '\n'; }      // destructor: runs when the object dies
};

int main() {
    std::cout << "start\n";
    {
        auto grass = std::make_unique<Texture>("grass.png");   // allocate and own
        std::cout << "  using " << grass->name << '\n';
    }   // grass goes out of scope here: the Texture is freed automatically
    std::cout << "after the block\n";
    auto rock = std::make_unique<Texture>("rock.png");
    std::cout << "end of main\n";
}   // rock freed here
