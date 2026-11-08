#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct SceneNode {
    std::string name;
    std::vector<std::unique_ptr<SceneNode>> children;
    explicit SceneNode(std::string n) : name(std::move(n)) {}
    ~SceneNode() { std::cout << "destroy " << name << '\n'; }
    SceneNode* add(std::string n) { children.push_back(std::make_unique<SceneNode>(std::move(n))); return children.back().get(); }
};

int main() {
    auto root = std::make_unique<SceneNode>("root");
    SceneNode* car = root->add("car");
    car->add("wheel");
    root->add("tree");
}
