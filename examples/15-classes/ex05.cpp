#include <iostream>

class Entity {
public:
    Entity() : id_(next_id_++) {}
    int id() const { return id_; }
    static int created() { return next_id_; }
private:
    int id_;
    static inline int next_id_ = 0;   // one shared counter for all Entity objects
};

int main() {
    Entity a, b, c;
    std::cout << a.id() << ' ' << b.id() << ' ' << c.id() << ", created " << Entity::created() << '\n';
}
