#include <iostream>
#include <memory>
#include <string>

struct Mesh {
    std::string name;
    explicit Mesh(std::string n) : name(std::move(n)) {}
    ~Mesh() { std::cout << "  mesh " << name << " freed\n"; }
};

int main() {
    std::shared_ptr<Mesh> tree_model;
    {
        auto m = std::make_shared<Mesh>("oak");
        std::cout << "count after make_shared: " << m.use_count() << '\n';
        tree_model = m;                                  // a second owner
        std::cout << "count after copy: " << m.use_count() << '\n';
    }   // m dies, count drops to 1: the mesh survives
    std::cout << "count after the block: " << tree_model.use_count() << '\n';
    tree_model.reset();                                  // last owner lets go: freed now
    std::cout << "end\n";
}
