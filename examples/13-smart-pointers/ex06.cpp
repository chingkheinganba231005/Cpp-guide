#include <iostream>
#include <memory>

struct Node {
    std::shared_ptr<Node> next;      // owns the next node
    std::weak_ptr<Node> prev;        // only observes the previous one: no cycle
    int value;
    explicit Node(int v) : value(v) {}
    ~Node() { std::cout << "  node " << value << " freed\n"; }
};

int main() {
    auto a = std::make_shared<Node>(1);
    auto b = std::make_shared<Node>(2);
    a->next = b;
    b->prev = a;
    if (auto p = b->prev.lock()) std::cout << "b's prev is " << p->value << '\n';   // lock() gives a temporary shared_ptr, or null if gone
    std::cout << "leaving main\n";
}
